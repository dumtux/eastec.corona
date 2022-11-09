# LED Light Controller with Solar MPTT Charging Controller and ESP32 BLE

## Quickstart

### Building Firmware

Clone the repository, init submodules.

```
$ git clone https://github.com/hotteshen/eastec.corona
$ git submodule init
$ git submodule update
$ ./idf.sh
$ cd firmware/
$ idf.py build
```

CAUTION: Do not run `idf.py set-target esp32c3` because it will overwrite `sdkconfig`, which is manually revised.

### Running Script to Communicate with the Device

Requires Bluetooth peripheral or USB dongle is present on the computer.

```
$ python3 -m venv devenv
$ source devenv/bin/activate
(devenv) $ pip install -r scripts/requirements.txt
(devenv) $ scripts/send_cmd.py --help
```

NOTE: Use another terminal session to activate the development environment. If using the same session wwhere IDF is activated, it will override IDF's virtual environment.
