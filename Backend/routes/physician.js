var express = require('express');
var Physician = require("../models/physician");
var User = require("../models/user");
const jwt = require("jwt-simple");
const fs = require('fs');

// On AWS ec2, you can use to store the secret in a separate file. 
// The file should be stored outside of your code directory. 
// For encoding/decoding JWT
const secretKey = fs.readFileSync(__dirname + '/../keys/jwtkey').toString();

var router = express.Router();

/*
All endpoints accessed via web interface
*/

//Create new physician
router.post("/createPhysician", function(req, res) {
    //Make sure email address isn't already linked to an account
    Physician.exists({email: {$eq: req.body.email}}, function(err, physician) {
        if (err) {
            res.status(500).send(err);
        }
        else if (physician) {
            res.status(409).json({error: "Email Already Linked to an Account!"});
        }
    });
    //Save new physician to database
    const newPhysician = new Physician({
        fullname: req.body.fullname,
        email: req.body.email,
        password: req.body.password
    });
    newPhysician.save(function (err, physician) {
        if (err) {
            res.status(500).send(err);
        }
        else {
            res.status(201).send();
        }
    });
});

//Login existing physician
router.post("/loginPhysician", function(req, res) {
    Physician.findOne({email: {$eq: req.body.email}}, function(err, physician) {
        if (err) {
            res.status(500).send(err);
        }
        else if (physician) {
            if (req.body.password === physician.password) {
                //Send back a token that contains the user's username
                const token = jwt.encode({username: req.body.email}, secretKey);
                res.status(200).json({ token: token });
            }
            else {
                res.status(401).json({error: "Bad username/password"});   
            }
        }
        else {
            res.status(401).json({error: "Bad username/password"});   
        }
    });
});

//Get existing physician data
router.post('/getPhysicianData', function(req, res) {
    //Check if the X-Auth header is set
    if (!req.headers["x-auth"]) {
        return res.status(401).json({error: "Missing X-Auth header"});
    }
    //X-Auth should contain the token value
    const token = req.headers["x-auth"];
    try {
        decoded = jwt.decode(token, secretKey);
        //Find the physician in the database
        Physician.findOne({email: {$eq: decoded.username}}, function(err, physician) {
            if (err) {
                res.status(500).send(err);
            }
            else {
                res.status(200).send(physician);  
            }
        });
    }
    catch (ex) {
        res.status(401).json({error: "Invalid JWT"});
    }
});

//Get all patient data
router.post('/physicianGetAllPatientData', function(req, res) {
    //Check if the X-Auth header is set
    if (!req.headers["x-auth"]) {
        return res.status(401).json({error: "Missing X-Auth header"});
    }
    //X-Auth should contain the token value
    const token = req.headers["x-auth"];
    try {
        decoded = jwt.decode(token, secretKey);
        //Find all users assigned to the physician in the database
        User.find({physician: {$eq: req.body.physician}}, function(err, users) {
            if (err) {
                res.status(500).send(err);
            }
            else {
                res.status(200).send(users);  
            }
        });
    }
    catch (ex) {
        res.status(401).json({error: "Invalid JWT"});
    }
});

//Get single patient data
router.post('/physicianGetSinglePatientData', function(req, res) {
    //Check if the X-Auth header is set
    if (!req.headers["x-auth"]) {
        return res.status(401).json({error: "Missing X-Auth header"});
    }
    //X-Auth should contain the token value
    const token = req.headers["x-auth"];
    try {
        decoded = jwt.decode(token, secretKey);
        //Find one user assigned to the physician in the database
        User.findOne({$and: [{physician: {$eq: req.body.physician}}, {email: {$eq: req.body.email}}]}, function(err, user) {
            if (err) {
                res.status(500).send(err);
            }
            else {
                res.status(200).send(user);
            }
        });
    }
    catch (ex) {
        res.status(401).json({error: "Invalid JWT"});
    }
});