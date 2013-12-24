# Drone browser


## Prerequisite
- nodejs
- npm 

## Setup
You can install/update the node dependencies 
```
npm install
```

Dependencies for client code are managed via bower 
```
npm install -g bower # install bower global mode
bower install
```

You need to have `ffmpeg` installed (it's used for the picture stuff)

To start server you have to install `coffeescript` for execute `Cakefile`

```
npm install -g coffescript
coffee app.coffee # after start the server open up http://localhost:3001
```


## Getting started
Connect to your ar drone 2 via WLAN, then run node.server.js open up localhost:3001 and you're ready to go.

Use `W, A, S, D` to move front, back and sideways. Use your `cursors` to go up/down or turn clockwise/counter clockwise.
Some animations can be triggered with `1-4`
`SPACE` for taking of and `esc` for landing.

When you crash use `e` to recover from emergency mode.

This project is heavily inspired from https://github.com/usefulthink/nodecopter-monitor . I just removed the three.js part and ported it from socket.io to faye since I like that better.

### Control via Curl (command line)

Now it is also available to control it by using curl. The reason using curl is that it is really simple to pass params from different languages. For example if you are using C++, you can execute the drone by using `curl http://localhost:3001/terminal/land/0.3` to land the drone

##### API format

```
/terminal/:action/:speed # :action means pass into action param, :speed means pass into speed param
```

- `action option` now support 
    - takeoff
    - front 
    - back 
    - up 
    - down 
    - left 
    - right 
    - clockwise 
    - counterClockwise
    - land

- `speed option` must be between 1 and 0

after you start the server(it should connect to `http://localhost:3001/` in default)

open up another terminal and enter:
```
curl http://localhost:3001/terminal/takeoff/0.3  # for takeoff
curl http://localhost:3001/terminal/land/0.3 # for landing
```

and you will see magic happened :)

##### Saving images

After you start the server, it will start saving images to `./tmp` file, if you dont have the folder the server will create one automatically.

The reason saving images is that, you can do lot of different fun project on it. Face detection, SURF detection, etc... cheers!

### SURF detection via openCV

If you want to start a detection via OpenCV you have to install OpenCV first, you will have to install cmake also to compile. After you have installed both OpenCV and cmake.
And enter following command

```
cd detect
make
cd build
make
./surf_detect
```

After you start surf detect, the program will start watching the file `/tmp`, and start SURF detection after picture saved in the folder. SURF detection will search for the object in `sample.jpeg` in `/detect` folder. So you will have to change `/detect/sample.jpeg` to the object that you wish to detect.
## Flow chart

![flow chart](https://raw.github.com/chilijung/drone-surf-detect/master/images/flow.png "chart")

## License 

MIT [@chilijung](chilijung@gmail.com)
