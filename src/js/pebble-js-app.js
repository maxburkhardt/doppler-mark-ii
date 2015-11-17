Pebble.addEventListener('ready', function(e) {
  console.log('JavaScript app ready and running!');
  Pebble.showSimpleNotificationOnPebble("test notification", "this is a test!");
});
