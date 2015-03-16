##Screenshot-util 

![screenshot-util](https://raw.githubusercontent.com/dezGusty/screenshot-util/master/res/icon.ico)
----------

###SYNOPSIS:

``screenshot[-wt WINDOW_TITLE | -rc LEFT TOP RIGHT BOTTOM | -mn MONITOR_VALUE |-o FILENAME | -h | -split]``

###OPTIONS :
``-wt     WINDOW_TITLE``                Select window with this title.

``-rc     LEFT TOP RIGHT BOTTOM``       Crop source.If no ``WINDOW_TITLE`` is provided
(0, 0) is the left top corner of desktop,
else if ``WINDOW_TITLE`` maches a desktop window
(0, 0) is its top left corner.

``-mn     MONITOR VALUE``  Specify what monitors the program should take a
screeenshot of. Separate values through commas
[ex. 1,2]. This option is ignored if ``WINDOW_TITLE``
is provided.

``-o FILENAME`` Output file name, if none, the image will be saved
as "screenshot.png" in the current working directory.

``-split``  Splits screenshots of different desktops (if any) into different pngs    

``-h`` Shows this help info.

###EXAMPLE USAGE


![ex1](https://raw.githubusercontent.com/dezGusty/screenshot-util/master/res/examples/ex1.PNG)

Called with no arguments, the program takes a screen shot of the main desktop and creates a file named "screenshot.png"

----------
![ex2](https://github.com/dezGusty/screenshot-util/blob/master/res/examples/ex2.png?raw=true)

The ``-wt`` argument allows you to specify the name of the window you want to capture. The ``-o`` 
argument allows you to specify the output folder. 

----------

![ex3](https://github.com/dezGusty/screenshot-util/blob/master/res/examples/ex3.png?raw=true)

Arguments with whitespace can also be given to the program using quotes 

----------

![ex4](https://github.com/dezGusty/screenshot-util/blob/master/res/examples/ex4.png?raw=true) 

The ``-rt`` argument allows you to specify a rectangle to capture, the order of the parameters is left, top , right, bottom. If the parameters exceed the size of the monitor and there is no other extending monitor the screenshot is filled with black in the undefined zones. If there is another monitor, the screenshot will extend to the second monitor. 

----------

![ex5](https://github.com/dezGusty/screenshot-util/blob/master/res/examples/ex5.png?raw=true)

``-mn`` argument allows you to specify which monitor(s) you want to take a screenshot of. The value of the main monitor is 1 and the value of the secondary monitors can be found at ``Control Panel\All Control Panel Items\Display\Screen Resolution``. The values of the monitors must be separated by a comma and no whitespace. The resulting screenshot splices the images of the desktops together. 

----------

![ex6](https://github.com/dezGusty/screenshot-util/blob/master/res/examples/ex6.png?raw=true) 

The ``-split`` argument can be used to create individual screenshots of each desktop. 

----------
