//Adding event listeners for the log in page nav bar:
$("#home").click(function(){
    $.get("/home");
});

$("#index").click(function(){
    $.get("/index");
});

$("#reference").click(function(){
    $.get("/reference");
});

//event listeners for the log in pages
$("#usrSignIn").click(function(){
    $.get("/users/loginUsr");
});

$("#physSignIn").click(function(){
    $.get("/physicians/loginPhysician");
});
