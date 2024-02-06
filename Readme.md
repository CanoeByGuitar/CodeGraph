# CodeGraph: Generate Function CallStack by code when writing blogs
## Usage (Only test in MacOS for now)
1. Build project (`mkdir -B build && cd build && make`)
2. Modify [description file](resources/callstack.txt)
3. Modify file name in main.py
4. `python3 main.py` and get screen capture to your blog
![img.png](resources%2Fimg.png)
![img_1.png](resources%2Fimg_1.png)

TODO:
* static reflection and front-side interpreter to support run-time changes
* more powerful usage need in blog writing