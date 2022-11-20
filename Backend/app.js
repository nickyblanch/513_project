/*
Require Modules
*/
var express = require('express');
const https = require("https");
const fs = require('fs');
/*var createError = require('http-errors');*/
var path = require('path');
var cookieParser = require('cookie-parser');
var logger = require('morgan');
const bodyParser = require('body-parser');
var indexRouter = require('./routes/index');
var userRouter = require('./routes/user');
var physicianRouter = require('./routes/physician');
var deviceRouter = require('./routes/device');
const { fstat } = require('fs');

//Create express app
var app = express();

/*
Setup Middleware
*/
// This is to enable cross-origin access
app.use(function (req, res, next) {
  // Website you wish to allow to connect
  res.setHeader('Access-Control-Allow-Origin', '*');
  // Request methods you wish to allow
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS, PUT, PATCH, DELETE');
  // Request headers you wish to allow
  res.setHeader('Access-Control-Allow-Headers', 'X-Requested-With,content-type');
  // Set to true if you need the website to include cookies in the requests sent
  // to the API (e.g. in case you use sessions)
  res.setHeader('Access-Control-Allow-Credentials', true);
  // Pass to next layer of middleware
  next();
});
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

/*
Setup Routers
*/
app.use('/', indexRouter);
app.use('/users', userRouter);
app.use('/physicians', physicianRouter);
app.use('/devices', deviceRouter);

/* catch 404 and forward to error handler
app.use(function(req, res, next) {
  next(createError(404));
});

// error handler
app.use(function(err, req, res, next) {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get('env') === 'development' ? err : {};
  // render the error page
  res.status(err.status || 500);
  res.render('error');
});
*/

https.createServer(
    {
        key: fs.readFileSync(__dirname + '/keys/key.pem'),
        cert: fs.readFileSync(__dirname + '/keys/cert.pem'),
    },
    app
)
.listen(3000, function() {
    console.log("Server Running on Port 3000...");
});

module.exports = app;
