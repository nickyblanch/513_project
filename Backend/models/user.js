const db = require("../db");

const userSchema = db.Schema({
    fullname: String,
    email: String,
    password: String,
    physician: String,
    devices: [ String ]
});

const User = db.model("User", userSchema);

module.exports = User;
