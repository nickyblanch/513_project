//Adding event listeners for the log in page nav bar:
$("#home").cick(function(){
    $.get("/home");
});

$("#index").cick(function(){
    $.get("/");
});

$("#reference").cick(function(){
    $.get("/reference");
});

//event listeners for the log in pages
$("#usrSignIn").cick(function(){
    $.get("/users/loginUsr");
});

$("#physSignIn").cick(function(){
    $.get("/physicians/loginPhysician");
});