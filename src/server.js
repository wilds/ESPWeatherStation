var https = require('https');

exports.handler = (request, context) => {
    if (request.directive.header.namespace === 'Alexa.Discovery' && request.directive.header.name === 'Discover') {
        log("DEBUG:", "Discover request",  JSON.stringify(request));
        handleDiscovery(request, context, "");
    }
    else if (request.directive.header.namespace === 'Alexa' || request.directive.endpoint.endpointId==='appliance-001') {
        if (request.directive.header.name === 'ReportState') {
            log("DEBUG:", "temperature Request", JSON.stringify(request));
            handleTemperature(request, context);
        }
    }

    function handleDiscovery(request, context) {
        var payload = {
            "endpoints": [
                {
                    "endpointId": "appliance-001",
                    "manufacturerName": "Wilds ESP device",
                    "friendlyName": "Smart Weatherstation",
                    "description": "Smart Weatherstation",
                    "displayCategories": ["TEMPERATURE_SENSOR"],
                    "cookie": {
                    },
                    "capabilities": [
                        {
                          "type": "AlexaInterface",
                          "interface": "Alexa",
                          "version": "3"
                        },
                        {
                            "type": "AlexaInterface",
                            "interface": "Alexa.TemperatureSensor",
                            "version": "3",
                            "properties": {
                                "supported": [
                                    {
                                        "name": "temperature"
                                    },
                                    {
                                        "name": "humidity"
                                    }
                                ],
                                "proactivelyReported": false,
                                "retrievable": true
                            }
                        }
                    ]
                }
            ]
        };
        var header = request.directive.header;
        header.name = "Discover.Response";
        log("DEBUG", "Discovery Response: ", JSON.stringify({ header: header, payload: payload }));
        context.succeed({ event: { header: header, payload: payload } });
    }

    function log(message, message1, message2) {
        console.log(message + message1 + message2);
    }

    function handleTemperature(request, context) {
        // get device ID passed in during discovery
        var requestMethod = request.directive.header.name;
        var responseHeader = request.directive.header;
        responseHeader.namespace = "Alexa";
        responseHeader.name = "StateReport";
        responseHeader.messageId = responseHeader.messageId + "-R";
        // get user token pass in request
        var requestToken = request.directive.endpoint.scope.token;
        var contextResult;

        //ThingSpeak Data Access
        var channel_id = process.env.channel_id;         //ThingSpeak Channel ID
        var speak_key = process.env.read_key;            //ThingSpeak Channel Read Key

        // Create URL to retrieve latest temperature reading from my ThingsSpeak channel (JSON format)
        var url = 'https://api.thingspeak.com/channels/' + channel_id + '/feed/last.json?api_key=' + speak_key;

        getTemperature(url).then(r => {

            if (requestMethod === "ReportState") {

                // Make the call to your device cloud for control
                // powerResult = stubControlFunctionToYourCloud(endpointId, token, request);
                contextResult = {
                    "properties": [
                        {
                            "namespace": "Alexa.TemperatureSensor",
                            "name": "temperature",
                            "value": {
                                "value": r.temperature,
                                "scale": "CELSIUS"
                            },
                            "timeOfSample": r.created_at, //retrieve from result.
                            "uncertaintyInMilliseconds": 1000
                        },
                        {
                            "namespace": "Alexa.TemperatureSensor",
                            "name": "humidity",
                            "value": {
                                "value": r.humidity,
                                "scale": "%"
                            },
                            "timeOfSample": r.created_at, //retrieve from result.
                            "uncertaintyInMilliseconds": 1000
                        }
                    ]
                };
            }

            var response = {
                context: contextResult,
                event: {
                    header: responseHeader,
                    endpoint: {
                        scope: {
                            type: "BearerToken",
                            token: requestToken
                        },
                        endpointId: "appliance-001"
                    },
                    payload: {}
                }
            };
            log("DEBUG", "Alexa.TemperatureSensor ", JSON.stringify(response));
            context.succeed(response);
        }, e => {
            context.fail("Got error: " + e);
        });
    }

    // Create the handler that responds to the Alexa Request.
    function getTemperature(url) {
        return new Promise((resolve, reject) => {
             https.get(url, (res) => {
                const body = [];
                res.on('data', (chunk) => body.push(chunk));
                res.on('end', () => {
                    var resp = JSON.parse(body.join(''));
                    resolve({
                        temperature: parseFloat(resp.field1),
                        humidity: parseFloat(resp.field2),
                        created_at: resp.created_at
                    });
                });
                res.on('error', (e) => {
                    reject(new Error(e.message));
                });
            });
        });
    };
};
