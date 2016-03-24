#include <pebble.h>

Window *my_window;
TextLayer *text_layer;
int StrokeStatus = 0;
int StrokeCount = 0;
CompassHeadingData compasdata;
long int heading = 999;
static char strokecounttext[20];

static void accel_data_handler(AccelData *data, uint32_t num_samples) {

   for (int i=0; i < (int)num_samples; i++){
   
      if(!data[i].did_vibrate) {
         // Print it out
//          APP_LOG(APP_LOG_LEVEL_DEBUG, "t: %lu, x: %d, y: %d, z: %d",
//                  (unsigned long)(data[i].timestamp),
//                  data[i].x,
//                  data[i].y,
//                  data[i].z);

         if ((data[i].y < 250) && (data[i].y > -250)) {
            //get compass data
            compass_service_peek(&compasdata);
            if (compasdata.compass_status == CompassStatusCalibrated) {
               heading = TRIGANGLE_TO_DEG(compasdata.magnetic_heading);
            } else {
               heading = 999;
            }
         }

         if(data[i].y > 500) {
            StrokeStatus = 0;
         } 
         if ((data[i].y < -500) && (StrokeStatus == 0)) {
            StrokeStatus = 1;
            StrokeCount++;
            snprintf(strokecounttext, sizeof(strokecounttext),"---%d---", StrokeCount);
            text_layer_set_text(text_layer, strokecounttext);
         }
        
                                                             
      } else {
         // Discard with a warning
         APP_LOG(APP_LOG_LEVEL_WARNING, "Vibration occured during collection");
      }
   }
   APP_LOG(APP_LOG_LEVEL_INFO, "=== %d = %ld ===", StrokeCount, heading);
}



void handle_init(void) {
   uint32_t num_samples = 5;  // Number of samples per batch/callback
   my_window = window_create();

   text_layer = text_layer_create(GRect(0, 0, 144, 20));
   text_layer_set_text(text_layer, "000");
   Layer *root_layer = window_get_root_layer(my_window);
   layer_add_child(root_layer, text_layer_get_layer(text_layer));

   
   window_stack_push(my_window, true);

   // Subscribe to batched data events
   accel_data_service_subscribe(num_samples, accel_data_handler);

//   // initialize compass and set a filter to 2 degrees
//   compass_service_set_heading_filter(DEG_TO_TRIGANGLE(2));
//   compass_service_subscribe(&compass_heading_handler);   
}

void handle_deinit(void) {
   text_layer_destroy(text_layer);
//    compass_service_unsubscribe();
   accel_data_service_unsubscribe();
   window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
