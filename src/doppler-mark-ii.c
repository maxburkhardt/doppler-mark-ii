#include <pebble.h>
#include <sample.h>

static Window *window;
static TextLayer *text_layer;
static BitmapLayer *doppler_data_layer;
static GBitmap *doppler_data;

static void window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Loading window");
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Debug info about maximum message sizes
  char max_inbox[16];
  snprintf(max_inbox, sizeof max_inbox, "%u", (unsigned int) app_message_inbox_size_maximum());
  char max_outbox[16];
  snprintf(max_outbox, sizeof max_outbox, "%u", (unsigned int) app_message_outbox_size_maximum());
  APP_LOG(APP_LOG_LEVEL_DEBUG, max_inbox);
  APP_LOG(APP_LOG_LEVEL_DEBUG, max_outbox);

  // Bitmap Layer
  doppler_data = gbitmap_create_with_resource(RESOURCE_ID_LOADING_SCREEN);
  doppler_data_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(doppler_data_layer, doppler_data);
  layer_add_child(window_layer, bitmap_layer_get_layer(doppler_data_layer));

  // TESTING: swap the bitmap layer
  GBitmap *new_data = gbitmap_create_from_png_data(sample, 3739);
  bitmap_layer_set_bitmap(doppler_data_layer, new_data);

  // Text Layer
  text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Hello, world");
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  gbitmap_destroy(doppler_data);
  bitmap_layer_destroy(doppler_data_layer);
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
