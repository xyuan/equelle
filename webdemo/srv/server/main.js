/* Libraries */
var cluster = require('cluster'),
    numCPUs = require('os').cpus().length,
    domain = require('domain'),
    http = require('http'),
    websocket = require('websocket').server,
    _ = require('underscore');

/* Own modules */
var helpers = require('./helpers.js'),
    socketHandler = require('./socketHandler.js');
    httpHandler = require('./httpHandler.js');

/* To take advantage of multi-core systems, we want to set up Node instances on half of the cores
   , so that the other long-running tasks have som room on the rest of them */
if (cluster.isMaster) {
    // If a worker is about to shut down, start a new one right away
    var shutdownWorkers = [];
    var handleWorkerMessage = function(msg) {
        if (msg.cmd == 'worker_shutdown') {
            console.log('Master got shutdown-message from pid:', msg.pid);
            shutdownWorkers.push(msg.pid);
            cluster.fork();
        }
    };

    // Fork worker instances
    var numWorkers = Math.max(Math.ceil(numCPUs/2),1);
    _.times(numWorkers, function() {
        var worker = cluster.fork();
        worker.on('message', handleWorkerMessage);
    });

    cluster.on('online', function(worker) {
        helpers.logInfo('Master', 'Worker '+worker.process.pid+' started');
    });

    // If a worker dies, log it, and start a new one
    cluster.on('exit', function(worker) {
        var pid = worker.process.pid;
        helpers.logError('Master', 'Worker '+pid+' died');

        if (_.contains(shutdownWorkers, pid)) {
            // We have already starte a new worker
            shutdownWorkers = _.without(shutdownWorkers, pid);
        } else {
            // Unexpected failure, star new worker
            var worker = cluster.fork();
            worker.on('message', handleWorkerMessage);
        }
    });

    helpers.logInfo('Master', 'Started, '+numWorkers+' workers forked');
} else {
    /* This is an actual server worker, each server runs a websocket server and a simple http server */
    var workerName = 'Worker '+process.pid;

    var workerSocketServer, workerHttpServer;

    /* Create the websocket server */
    var sockDomain = domain.create();
    var openSockets = [];
    sockDomain.run(function() {
        var handlerName = workerName+' Socket HTTP Server';

        // Start the http-server
        var httpServer = workerSocketServer = http.createServer(function(req,res) {
            // Respond to all non-websocket requests with a 404
            res.writeHead(404);
            res.end();
        }).listen(8888, function() {
            helpers.logInfo(workerName, 'Started socket HTTP server');
        });

        // Configure the WebSocket server
        var socketServer = new websocket({
            httpServer: httpServer,
            autoAcceptConnections: false,
            maxReceivedFrameSize: 10*1024*1024, //10MiB
            maxReceivedMessageSize: 10*1024*1024 //10MiB
        });

        // Setup connection handling
        socketServer.on('request', function(req) {
            if (socketHandler.shouldAccept(req.origin) && req.requestedProtocols.length == 1) {
                // Client is allowed to connect, find out what it wants to do
                var protocol = req.requestedProtocols[0];
                var handler = socketHandler.acceptHandler(protocol);
                if (typeof handler == 'function') {
                    // Accept connection from client
                    var connection = req.accept(protocol, req.origin);
                    // Add a sendJSON method for later use
                    connection.sendJSON = function(obj) { connection.sendUTF(JSON.stringify(obj)) };

                    // Add to open connections
                    openSockets.push(connection);
                    connection.on('close', function() { openSockets = _.without(openSockets, connection) });

                    // Setup a new error-handling domain for this connection
                    var d = domain.create();
                    d.add(req);
                    d.add(connection);

                    var abort;

                    d.on('error', function(error) {
                        var msg = 'Error in handling connection to :"'+protocol+'" : '+error;
                        helpers.logError(handlerName, msg);

                        // Drop current connection
                        connection.drop();

                        // Abort process
                        if (abort) abort(msg);

                        // Shut down worker
                        sockDomain.emit('error', new Error('Error in request-handling'));
                    });

                    // Handle this connection
                    d.run(function() {
                        try {
                            var doHandle = function() {
                                abort = handler(handlerName, d, connection, doHandle);
                            };
                            doHandle();
                        } catch (e) {
                            d.emit('error', e);
                        }
                    });

                    // If the connection is closed, abort the process
                    connection.on('close', function() {
                        if (abort) abort('Connection closed');
                    });
                } else {
                    req.reject();
                }
            } else {
                req.reject();
            }
        });
    });

    /* Create the httpserver */
    var httpDomain = domain.create();
    var openRequests = [];
    httpDomain.run(function() {
        var handlerName = workerName+' Socket HTTP Server';

        // Start the http-server
        var httpServer = workerHttpServer = http.createServer(function(req, res) {
            // Setup a new error-handling domain for this request
            var d = domain.create();
            d.add(req);
            d.add(res);

            // Add to open connections
            openRequests.push(req);
            res.on('finish', function() { openRequests = _.without(openRequests, req) });

            d.on('error', function(error) {
                helpers.logError(handlerName, 'Error in handling request to :"'+req.url+'" : '+error);

                // End current request with an error
                res.writeHead(500);
                res.end();

                // Shut down worker
                httpDomain.emit('error', new Error('Error in request-handling'));
            });

            // Handle this connection
            d.run(function() {
                try {
                    httpHandler.handleRequest(handlerName, d, req, res);
                } catch (e) {
                    d.emit('error', e);
                }
            });
        }).listen(8887, function() {
            helpers.logInfo(workerName, 'Started HTTP server');
        });
    });

    /* Setup error handling if either of the websocket or http-server breaks down */
    var shutdownWorker = function() {
        helpers.logInfo(workerName, 'Shutting down...');

        // Stop handlers from accepting any more requests
        workerSocketServer.close();
        workerHttpServer.close();

        // Kill this process when all current requests are done
        var killcheck = setInterval(function() {
            if (openRequests.length == 0 && openSockets.length == 0) {
                process.exit(1);
            }
        }, 1000);
        // But don't keep the process open just for that!
        killcheck.unref();

        // Kill this process if it does not shut down by itself within 30 minutes
        var killtimer = setTimeout(function() {
            process.exit(1);
        }, 30*1000*60);
        // But don't keep the process open just for that!
        killtimer.unref();

        // Let the master know we are shutting down, so it can start a new instance in advance
        process.send({ cmd: 'worker_shutdown', pid: process.pid });
    };

    sockDomain.on('error', function(error) {
        helpers.logError(workerName, 'Socket HTTP server error: '+error);
        shutdownWorker();
    });
    httpDomain.on('error', function(error) {
        helpers.logError(workerName, 'HTTP server error: '+error);
        shutdownWorker();
    });

}
