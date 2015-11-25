#include <pebble.h>

#define KEY_IMAGE 0
#define KEY_LENGTH 1

static Window *window;
static TextLayer *text_layer;
static BitmapLayer *doppler_data_layer;
static GBitmap *doppler_data;
static uint8_t *old_image;
static uint8_t *new_image;

static void window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Loading window");
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Debug info about maximum message sizes
  /*
  char max_inbox[16];
  snprintf(max_inbox, sizeof max_inbox, "%u", (unsigned int) app_message_inbox_size_maximum());
  char max_outbox[16];
  snprintf(max_outbox, sizeof max_outbox, "%u", (unsigned int) app_message_outbox_size_maximum());
  APP_LOG(APP_LOG_LEVEL_DEBUG, max_inbox);
  APP_LOG(APP_LOG_LEVEL_DEBUG, max_outbox);
  */

  // Bitmap Layer
  doppler_data = gbitmap_create_with_resource(RESOURCE_ID_LOADING_SCREEN);
  doppler_data_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(doppler_data_layer, doppler_data);
  layer_add_child(window_layer, bitmap_layer_get_layer(doppler_data_layer));
  // We create a dummy new_image to make memory management easier
  new_image = (uint8_t*)malloc(1);

  // Text Layer
  text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "00:00");
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  gbitmap_destroy(doppler_data);
  bitmap_layer_destroy(doppler_data_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received!");
  Tuple *length = dict_find(iterator, KEY_LENGTH);
  Tuple *data = dict_find(iterator, KEY_IMAGE);
  int image_len = (int)length->value->int16;
  char image_len_str[16];
  snprintf(image_len_str, sizeof image_len_str, "%d", image_len);
  APP_LOG(APP_LOG_LEVEL_INFO, image_len_str);
  old_image = new_image;
  new_image = (uint8_t*)malloc(image_len);
  memcpy(new_image, data->value->data, image_len);
  gbitmap_destroy(doppler_data);
  doppler_data = gbitmap_create_from_png_data(new_image, image_len);
  bitmap_layer_set_bitmap(doppler_data_layer, doppler_data);
  free(old_image);
}

static void request_new_image() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int value = 1;
  dict_write_int(iter, 0, &value, sizeof(int), true);
  app_message_outbox_send();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  static char s_time_buffer[16];
  if (clock_is_24h_style()) {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
  } else {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%I:%M", tick_time);
  }
  text_layer_set_text(text_layer, s_time_buffer);

  // Reload the map every 5 minutes
  if (tick_time->tm_min % 5 == 0) {
    request_new_image();
  }
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorWhite);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  // Initialize message receipt
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_register_inbox_received(inbox_received_callback);

  // Initialize tick handler
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
