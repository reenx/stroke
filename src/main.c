#include <pebble.h>

Window *my_window;
TextLayer *stroke_count_layer;
TextLayer *lap_count_layer;
TextLayer *stroke_time_layer;
int StrokeStatus = 0;
int StrokeCount = 0;
int LapCount = 0;
CompassHeadingData compasdata;
long int heading = 999;
static char strokecounttext[5];
static char lapcounttext[10];
static char stroketimetext[11];
time_t lap_seconds = 0;
uint16_t lap_milliseconds = 0;
time_t new_seconds = 0;
uint16_t new_milliseconds = 0;
time_t prev_seconds = 0;
uint16_t prev_milliseconds = 0;

uint32_t time_diff_ms(time_t base_seconds, uint16_t base_milliseconds) {
   uint32_t diff_milliseconds = 0;
   time_t diff_seconds = 0 ;
   if (new_seconds > base_seconds){
      diff_seconds  = new_seconds - base_seconds;
   }
   diff_milliseconds = diff_seconds * 1000;
   diff_milliseconds = diff_milliseconds + new_milliseconds;
   if (diff_milliseconds > base_milliseconds){
      diff_milliseconds = diff_milliseconds - base_milliseconds;
   } else {
      diff_milliseconds = 0;
   }
   return diff_milliseconds;
}

static void accel_data_handler(AccelData *data, uint32_t num_samples) {
   
   time_ms(&new_seconds, &new_milliseconds);
   if (prev_seconds == 0) {
      prev_seconds = new_seconds;
      prev_milliseconds = new_milliseconds;
      lap_seconds = new_seconds;
      lap_milliseconds = new_milliseconds;
   }
   uint32_t laptime = time_diff_ms(lap_seconds, lap_milliseconds);
//    APP_LOG(APP_LOG_LEVEL_INFO, "=== %ld = %d ===", lap_seconds, lap_milliseconds);

   snprintf(stroketimetext, sizeof(stroketimetext),"%li", laptime);
   text_layer_set_text(stroke_time_layer, stroketimetext);

   for (int i=0; i < (int)num_samples; i++){
   
      if(!data[i].did_vibrate) {
         // Print it out
//          APP_LOG(APP_LOG_LEVEL_DEBUG, "t: %lu, x: %d, y: %d, z: %d",
//                  (unsigned long)(data[i].timestamp),
//                  data[i].x,
//                  data[i].y,
//                  data[i].z);


         if(data[i].y > 500) {
            StrokeStatus = 0;
         } 
         if ((data[i].y < -500) && (StrokeStatus == 0)) {
            StrokeStatus = 1;
            StrokeCount++;
            snprintf(strokecounttext, sizeof(strokecounttext),"%d", StrokeCount);
            text_layer_set_text(stroke_count_layer, strokecounttext);
            if (prev_milliseconds != 0) {
               uint32_t stroketime = time_diff_ms(prev_seconds, prev_milliseconds);
               
               if ((stroketime < 1000) && (laptime >10000)){
                  lap_seconds = new_seconds;
                  lap_milliseconds = new_milliseconds;
                  LapCount++;
                  snprintf(lapcounttext, sizeof(lapcounttext),"%d", LapCount);
                  text_layer_set_text(lap_count_layer, lapcounttext);
               }
                  
            }
            prev_seconds = new_seconds;
            prev_milliseconds = new_milliseconds;
         }
        
                                                             
      } else {
         // Discard with a warning
         APP_LOG(APP_LOG_LEVEL_WARNING, "Vibration occured during collection");
      }
   }
   APP_LOG(APP_LOG_LEVEL_INFO, "=== %d ===", StrokeCount);
}



void handle_init(void) {
   uint32_t num_samples = 5;  // Number of samples per batch/callback
   my_window = window_create();

   stroke_count_layer = text_layer_create(GRect(5, 5, 135, 50));
   stroke_time_layer = text_layer_create(GRect(5, 60, 135, 50));
   lap_count_layer = text_layer_create(GRect(5, 115, 135, 50));
   text_layer_set_text(stroke_count_layer, "000");
   text_layer_set_text(stroke_time_layer, "0000");
   text_layer_set_text(lap_count_layer, "0");
   text_layer_set_text_alignment(stroke_count_layer, GTextAlignmentRight);
   text_layer_set_text_alignment(stroke_time_layer, GTextAlignmentRight);
   text_layer_set_text_alignment(lap_count_layer, GTextAlignmentRight);
   text_layer_set_font(stroke_count_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
   text_layer_set_font(stroke_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
   text_layer_set_font(lap_count_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
   text_layer_set_background_color(stroke_count_layer, GColorYellow);
   text_layer_set_background_color(lap_count_layer, GColorYellow);
   text_layer_set_background_color(stroke_time_layer, GColorGreen);
   Layer *root_layer = window_get_root_layer(my_window);
   layer_add_child(root_layer, text_layer_get_layer(stroke_count_layer));
   layer_add_child(root_layer, text_layer_get_layer(lap_count_layer));
   layer_add_child(root_layer, text_layer_get_layer(stroke_time_layer));

   
   window_stack_push(my_window, true);

   // Subscribe to batched data events
   accel_data_service_subscribe(num_samples, accel_data_handler);

//   // initialize compass and set a filter to 2 degrees
//   compass_service_set_heading_filter(DEG_TO_TRIGANGLE(2));
//   compass_service_subscribe(&compass_heading_handler);   
}

void handle_deinit(void) {
   text_layer_destroy(stroke_count_layer);
//    compass_service_unsubscribe();
   accel_data_service_unsubscribe();
   window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
