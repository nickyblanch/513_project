//Connect to our instance of MongoDB
const mongoose = require("mongoose");
mongoose.connect("mongodb://localhost/ece513finalproject", { useNewUrlParser: true, useUnifiedTopology:true });
module.exports = mongoose;