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
        res.status(401).json({error: "Invalid JWT"});
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
        decoded = jwt.decode(token, secretKey);
        //Make sure device actually exists
        Device.exists({id: {$eq: req.body.id}}, function(err, device) {
            if (err) {
                res.status(500).send(err);
            }
            else if (!device) {
                res.status(400).json({error: "Device Does Not Exist"});
            }
        });
        //Remove the device from the user's list of devices
        User.findOneAndUpdate({email: {$eq: decoded.username}}, 
        {$pull: {devices: req.body.id}}, null, function(err, user) {
            if (err) {
                res.status(500).send(err);
            }
        });
        //Delete the device from the database
        Device.deleteOne({id: {$eq: req.body.id}}, function(err, device) {
            if (err) {
                res.status(500).send(err);
            }
            else {
                res.status(204).send();
            }
        });
    }
    catch (ex) {
        res.status(401).json({error: "Invalid JWT"});
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
    try {
        decoded = jwt.decode(token, secretKey);
        //Find the device in the database
        Device.findOneAndUpdate({id: {$eq: req.body.id}}, 
        {rate: req.body.rate, rangeStart: req.body.rangeStart, rangeEnd: req.body.rangeEnd}, null, function(err, device) {
            if (err) {
                res.status(500).send(err);
            }
            else {
                res.status(204).send();
            }
        });
    }
    catch (ex) {
        res.status(401).json({error: "Invalid JWT"});
    }
});

//Read existing device info
router.post('/readDevice', function(req, res) {
    //Check if the X-Auth header is set
    if (!req.headers["x-auth"]) {
        return res.status(401).json({error: "Missing X-Auth header"});
    }
    //X-Auth should contain the token value
    const token = req.headers["x-auth"];
    try {
        decoded = jwt.decode(token, secretKey);
        //Find the device in the database
        Device.findOne({id: {$eq: req.body.id}}, function(err, device) {
            if (err) {
                res.status(500).send(err);
            }
            else {
                res.status(200).send(device);
            }
        });
    }
    catch (ex) {
        res.status(401).json({error: "Invalid JWT"});
    }
});

/*
Endpoints accessed by the IoT device
*/

//Record reading
router.post('/recordReading', function(req, res) {
    Device.findOne({id: {$eq: req.body.id}}, function(err, device) {
        if (err) {
            res.status(500).send(err);
        }
        else if (device) {
            if (req.body.APIkey === device.APIkey) {
                var recording = {recording: Date.now(), heartRate: req.body.heartRate, bosLevel: req.body.bosLevel};
                Device.findOneAndUpdate({id: {$eq: req.body.id}},
                {$push: {recordings: recording}}, null, function(err, device) {
                    if (err) {
                        res.status(500).send(err);
                    }
                    else {
                        res.status(204).send();
                    }
                });
            }
            else {
                res.status(401).send();
            }
        }
        else {
            res.status(400).json({error: "Invalid Device ID"});
        }
    });
});

//Pull device parameters
router.post('/pullParameters', function(req, res) {
    Device.findOne({id: {$eq: req.body.id}}, function(err, device) {
        if (err) {
            res.status(500).send(err);
        }
        else if (device) {
            if (req.body.APIkey === device.APIkey) {
                res.status(200).send(device);
            }
            else {
                res.status(401).send();
            }
        }
        else {
            res.status(400).json({error: "Invalid Device ID"});
        }
    });
});

module.exports = router;