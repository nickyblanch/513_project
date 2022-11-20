var express = require('express');
var User = require("../models/user")
var Device = require("../models/device")
const jwt = require("jwt-simple");
const crypto = require('crypto');
const fs = require('fs');

// On AWS ec2, you can use to store the secret in a separate file. 
// The file should be stored outside of your code directory. 
// For encoding/decoding JWT
const physicianKey = fs.readFileSync(__dirname + '/../keys/physiciankey').toString();
const userKey = fs.readFileSync(__dirname + '/../keys/userkey').toString();

var router = express.Router();

//Function for generating API Keys
function generateKey(size = 32, format = 'base64') {
    const buffer = crypto.randomBytes(size);
    return buffer.toString(format);
}

/*
Endpoints accessed by the web interface
*/

//Register a new device
router.post("/registerDevice", function(req, res) {
    //Check if the X-Auth header is set
    if (!req.headers["x-auth"]) {
        return res.status(401).json({error: "Missing X-Auth header"});
    }
    //X-Auth should contain the token value
    const token = req.headers["x-auth"];
    try {
        decoded = jwt.decode(token, userKey);
        //Make sure device has not already been registered
        Device.exists({deviceID: {$eq: req.body.deviceID}}, function(err, device) {
            if (err) {
                return res.status(500).send(err);
            }
            else if (device) {
                return res.status(409).json({error: "Device Already Registered"});
            }
            else {
                //Add the device to the user's list of devices
                User.findOneAndUpdate({email: {$eq: decoded.username}},
                {$push: {devices: req.body.deviceID}}, null, function(err, user) {
                    if (err) {
                        return res.status(500).send(err);
                    }
                    else {
                        //Create the new device in the database with a unique API key
                        const newDevice = new Device({
                            deviceID: req.body.deviceID,
                            APIkey: generateKey(),
                            rate: 30,
                            rangeStart: 6,
                            rangeEnd: 20,
                            recordings: []
                        });
                        newDevice.save(function (err, device) {
                            if (err) {
                                return res.status(500).send(err);
                            }
                            else {
                                return res.status(201).send(device);
                            }
                        });
                    }
                });
            }
        });
    }
    catch (ex) {
        return res.status(401).json({error: "Invalid JWT"});
    }
});

//Delete existing device
router.post('/deleteDevice', function(req, res) {
    //Check if the X-Auth header is set
    if (!req.headers["x-auth"]) {
        return res.status(401).json({error: "Missing X-Auth header"});
    }
    //X-Auth should contain the token value
    const token = req.headers["x-auth"];
    try {
        decoded = jwt.decode(token, userKey);
        //Remove the device from the user's list of devices
        User.findOneAndUpdate({email: {$eq: decoded.username}}, 
        {$pull: {devices: req.body.deviceID}}, null, function(err, user) {
            if (err) {
                return res.status(500).send(err);
            }
            else {
                //Delete the device from the database
                Device.deleteOne({deviceID: {$eq: req.body.deviceID}}, function(err, device) {
                    if (err) {
                        return res.status(500).send(err);
                    }
                    else {
                        return res.status(204).send();
                    }
                });
            }
        });
    }
    catch (ex) {
        return res.status(401).json({error: "Invalid JWT"});
    }
});

//Update device parameters
router.post('/updateDevice', function(req, res) {
    //Check if the X-Auth header is set
    if (!req.headers["x-auth"]) {
        return res.status(401).json({error: "Missing X-Auth header"});
    }
    //X-Auth should contain the token value
    const token = req.headers["x-auth"];
    //Try to authenticate user
    try {
        decoded = jwt.decode(token, userKey);
        //Find and update the device in the database
        Device.findOneAndUpdate({deviceID: {$eq: req.body.deviceID}}, 
        {rate: req.body.rate, rangeStart: req.body.rangeStart, rangeEnd: req.body.rangeEnd}, null, function(err, device) {
            if (err) {
                return res.status(500).send(err);
            }
            else {
                return res.status(204).send();
            }
        });
    }
    //Otherwise, try to authenticate physician
    catch (ex) {
        try {
            decoded = jwt.decode(token, physicianKey);
            //Find and update the device in the database
            Device.findOneAndUpdate({deviceID: {$eq: req.body.deviceID}}, 
            {rate: req.body.rate, rangeStart: req.body.rangeStart, rangeEnd: req.body.rangeEnd}, null, function(err, device) {
                if (err) {
                    return res.status(500).send(err);
                }
                else {
                    return res.status(204).send();
                }
            });
        }
        //Else, unauthenticated
        catch (ex) {
            return res.status(401).json({error: "Invalid JWT"});
        }
    }
});

//Read device info
router.post('/readDevice', function(req, res) {
    //Check if the X-Auth header is set
    if (!req.headers["x-auth"]) {
        return res.status(401).json({error: "Missing X-Auth header"});
    }
    //X-Auth should contain the token value
    const token = req.headers["x-auth"];
    //Try to authenticate user
    try {
        decoded = jwt.decode(token, userKey);
        //Find the device in the database
        Device.findOne({deviceID: {$eq: req.body.deviceID}}, function(err, device) {
            if (err) {
                return res.status(500).send(err);
            }
            else {
                return res.status(200).send(device);
            }
        });
    }
    //Otherwise, try to authenticate physician
    catch (ex) {
        try {
            decoded = jwt.decode(token, userKey);
            //Find the device in the database
            Device.findOne({deviceID: {$eq: req.body.deviceID}}, function(err, device) {
                if (err) {
                    return res.status(500).send(err);
                }
                else {
                    return res.status(200).send(device);
                }
            });
        }
        catch (ex) {
            return res.status(401).json({error: "Invalid JWT"});
        }
    }
});

/*
Endpoints accessed by the IoT device
*/

//Record reading
router.post('/recordReading', function(req, res) {
    //Check if the X-API-Key header is set
    if (!req.headers["x-api-key"]) {
        return res.status(401).json({error: "Missing X-API-Key header"});
    }
    //Record sensor reading
    var recording = {recording: Date.now(), heartRate: req.body.heartRate, bosLevel: req.body.bosLevel};
    Device.findOneAndUpdate({APIkey: {$eq: req.headers["x-api-key"]}},
    {$push: {recordings: recording}}, null, function(err, device) {
        if (err) {
            return res.status(500).send(err);
        }
        if (!device) {
            return res.status(401).json({error: "Invalid API key"});
        }
        else {
            return res.status(204).send();
        }
    });
});

//Pull device parameters
router.post('/pullParameters', function(req, res) {
    //Check if the X-API-Key header is set
    if (!req.headers["x-api-key"]) {
        return res.status(401).json({error: "Missing X-API-Key header"});
    }
    //Read out current device parameters
    Device.findOne({APIkey: {$eq: req.headers["x-api-key"]}}, function(err, device) {
        if (err) {
            return res.status(500).send(err);
        }
        if (!device) {
            return res.status(401).json({error: "Invalid API key"});
        }
        else {
            return res.status(200).json({rate: device.rate, rangeStart: device.rangeStart, rangeEnd: device.rangeEnd});
        }
    });
});

module.exports = router;