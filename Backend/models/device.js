const db = require("../db");

const deviceSchema = db.Schema({
    id: Number,
    APIkey: String,
    rate: Number,
    rangeStart: Number, 
    rangeEnd: Number,
    recordings: [
        {
            recording: Date,
            heartRate: Number,
            bosLevel: Number
        }
    ]
});

const Device = db.model("Device", deviceSchema);

module.exports = Device;