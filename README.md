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
