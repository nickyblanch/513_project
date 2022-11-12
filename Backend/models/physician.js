const db = require("../db");

const physicianSchema = db.Schema({
    fullname: String,
    email: String,
    password: String
});

const Physician = db.model("Physician", physicianSchema);

module.exports = Physician;