var express = require('express');
var User = require("../models/user");
var Physician = require("../models/physician");
const jwt = require("jwt-simple");
const fs = require('fs');

// On AWS ec2, you can use to store the secret in a separate file. 
// The file should be stored outside of your code directory. 
// For encoding/decoding JWT
const userKey = fs.readFileSync(__dirname + '/../keys/userkey').toString();

var router = express.Router();

/*
All endpoints accessed via web interface
*/

//Create new user
router.post("/createUser", function(req, res) {
    //Make sure email address isn't already linked to an account
    User.exists({email: {$eq: req.body.email}}, function(err, user) {
        if (err) {
            return res.status(500).send(err);
        }
        else if (user) {
            return res.status(409).json({error: "Email Already Linked to an Account!"});
        }
        else {
            //Save new user to database
            const newUser = new User({
                fullname: req.body.fullname,
                email: req.body.email,
                password: req.body.password,
                physician: "none",
                devices: []
            });
            newUser.save(function (err, user) {
                if (err) {
                    return res.status(500).send(err);
                }
                else {
                    return res.status(201).send(user);
                }
            });
        }
    });
});

//Login existing user
router.post("/loginUser", function(req, res) {
    //Validate user login using email and password
    User.findOne({email: {$eq: req.body.email}}, function(err, user) {
        if (err) {
            return res.status(500).send(err);
        }
        else if (user) {
            if (req.body.password === user.password) {
                //Send back a token that contains the user's username
                const token = jwt.encode({username: req.body.email}, userKey);
                return res.status(200).json({ token: token });
            }
            else {
                return res.status(401).json({error: "Bad username/password"});   
            }
        }
        else {
            return res.status(401).json({error: "Bad username/password"});   
        }
    });
});

//Get current user data
router.post('/getUserData', function(req, res) {
    //Check if the X-Auth header is set
    if (!req.headers["x-auth"]) {
        return res.status(401).json({error: "Missing X-Auth header"});
    }
    //X-Auth should contain the token value
    const token = req.headers["x-auth"];
    try {
        decoded = jwt.decode(token, userKey);
        //Find the user in the database
        User.findOne({email: {$eq: decoded.username}}, function(err, user) {
            if (err) {
                return res.status(500).send(err);
            }
            else {
                return res.status(200).send(user);  
            }
        });
    }
    catch (ex) {
        return res.status(401).json({error: "Invalid JWT"});
    }
});

//Update existing user data
router.post('/updateUserData', function(req, res) {
    //Check if the X-Auth header is set
    if (!req.headers["x-auth"]) {
        return res.status(401).json({error: "Missing X-Auth header"});
    }
    //X-Auth should contain the token value
    const token = req.headers["x-auth"];
    try {
        decoded = jwt.decode(token, userKey);
        //Find and update the user in the database
        User.findOneAndUpdate({email: {$eq: decoded.username}},
        {$set: {fullname: req.body.fullname, password: req.body.password}}, null, function(err, user) {
            if (err) {
                return res.status(500).send(err);
            }
            else {
                return res.status(204).send(); 
            }
        });
    }
    catch (ex) {
        return res.status(401).json({error: "Invalid JWT"});
    }
});

//List all physicians by fullname
router.post('/listPhysicians', function(req, res) {
    //Check if the X-Auth header is set
    if (!req.headers["x-auth"]) {
        return res.status(401).json({error: "Missing X-Auth header"});
    }
    //X-Auth should contain the token value
    const token = req.headers["x-auth"];
    try {
        decoded = jwt.decode(token, userKey);
        //Find all physicians and return their fullnames
        Physician.find({}, function(err, physicians) {
            if (err) {
                return res.status(500).send(err);
            }
            else {
                var physicianList = [];
                physicians.forEach(physician => {
                    physicianList.push(physician.fullname);
                });
                return res.status(200).json({physicians: physicianList});
            }
        });
    }
    catch (ex) {
        return res.status(401).json({error: "Invalid JWT"});
    }
});

//Assign physician to user
router.post('/assignPhysician', function(req, res) {
    //Check if the X-Auth header is set
    if (!req.headers["x-auth"]) {
        return res.status(401).json({error: "Missing X-Auth header"});
    }
    //X-Auth should contain the token value
    const token = req.headers["x-auth"];
    try {
        decoded = jwt.decode(token, userKey);
        //Verify physician actually exists
        Physician.exists({fullname: {$eq: req.body.physician}}, function(err, physician) {
            if (err) {
                return res.status(500).send(err);
            }
            else if (!physician) {
                return res.status(400).json({error: "Physician Does not Exist"});
            }
            else {
                //Assign the physician to the user
                User.findOneAndUpdate({email: {$eq: decoded.username}},
                {$set: {physician: req.body.physician}}, null, function(err, user) {
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

module.exports = router;