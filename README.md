brainblast
==========

brainf*ck for GUI

This works on Win32.

## How to build

It requires VC++.

    nmake

## commands

`ptr` is data pointer.

* \>\<+-[]

    There are the same as Brainf*ck.  `.` and `,` is not exist.

* $

    store coordinates of the mouse to `ptr`.  
    `ptr = x`  
    `(ptr + 1) = y`

* #

    draw the dot to (x, y) by `color`.  
    `(x, y) = (ptr, ptr + 1)`

* @

    store whether mouse is clicked to `ptr`. If mouse is clicked, it stores `1`, else it stores `0`.  
    `ptr = left_clicked`  
    `(ptr + 1) = right_clicked`

* !

    show messagebox  
    `message = ptr(as char)...`

* &

    show image(./image.bmp) to (x, y). The dot in (0, 0) is treaded transparent color.  
    `(x, y) = (ptr, ptr + 1)`

* %

    save as bitmap image file(./out.bmp)
