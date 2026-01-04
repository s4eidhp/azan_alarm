# Azan Alarm for Linux (Raspberry Pi)

This application fetches prayer times for Tehran from the API, schedules Azan playback for Dhuhr and Maghrib using cron, and plays an MP3 file using mpg123.

## Requirements
- curl (preinstalled on most systems)
- mpg123 (install with `sudo apt install mpg123`)
- azan.mp3 file in the same directory

## Building
make

## Usage
- Run `./main` to fetch times and schedule cron jobs.
- Run `./main show` to display all prayer times.
- Run `./main -h` or `./main --help` for help.
- The app will add cron jobs for Dhuhr and Maghrib.
- To run on boot, add to crontab: `@reboot sleep 30 && /full/path/to/main` (waits 30 seconds for network)
- Alternatively, run the program manually after boot

## Notes
- Assumes mpg123 is installed for MP3 playback.
- Cron jobs are added to the user's crontab.
