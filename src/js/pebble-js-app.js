var locationOptions = {
  enableHighAccuracy: true, 
  maximumAge: 10000, 
  timeout: 10000
};

var req = new XMLHttpRequest();
req.addEventListener("load", imageGetHandler);
req.responseType = "arraybuffer";

function getDopplerImage(pos) {
  console.log("Getting doppler image");
  var latitude = pos.coords.latitude;
  var longitude = pos.coords.longitude;
  req.open("GET", "https://files.armada.systems/doppler.png?latitude=" + latitude + "&longitude=" + longitude);
  req.send();
}

function locationError(err) {
  console.log("Could not get location.");
}

function imageGetHandler(resp) {
  console.log("Image downloaded!");
  var imageBlob = req.response;
  if (imageBlob) {
    var imageArray = new Uint8Array(imageBlob);
    console.log("Length of blob: " + imageArray.byteLength);
    var sendArray = [];
    for(var i=0; i<imageArray.byteLength; i++) {
      sendArray.push(imageArray[i]);
    }
    Pebble.sendAppMessage({'KEY_LENGTH': imageArray.byteLength, 'KEY_IMAGE': sendArray});
  }
}

Pebble.addEventListener('ready', function(e) {
  console.log('JavaScript app ready and running!');

  //find our location
  navigator.geolocation.getCurrentPosition(getDopplerImage, locationError, locationOptions);
});
