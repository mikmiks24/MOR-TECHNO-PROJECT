// Copy this file to config.js and fill in your Supabase project values.
// config.js is git-ignored — keep your keys out of source control.
//
// Use the SAME project as the ESP32 firmware (config.h). The anon (public)
// key is safe to use in the browser as long as Row Level Security is enabled
// on the staff / terminals / logs tables (see supabase/schema.sql).
//
// If this file is missing or left as the placeholder, dashboard.html falls
// back to built-in demo data so the UI still renders.

window.ECS_CONFIG = {
  SUPABASE_URL: "https://YOUR_PROJECT.supabase.co",
  SUPABASE_ANON_KEY: "YOUR_SUPABASE_ANON_KEY"
};
