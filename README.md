
## Thư viện chuẩn hóa văn bản Tiếng Việt (Có sẵn wrapper cho Python)




Installation
------------

- Build
```sh

#Require python_dev and g++ 


# For Python2  Need setup path to python2_dev include and g++ 
#apt install python2-dev
#apt install g++
export PYTHON2_DEV_INCULE=/usr/include/python2.7
export GPP_COMPILER=g++
cp -f MakefilePython2 Makefile
make -j



# For Python3 : Need setup path to python3_dev include and g++ 
# sudo apt install python3-dev
# sudo apt install g++
export PYTHON3_DEV_INCULE=/usr/include/python3.8
export GPP_COMPILER=g++
cp -f MakefilePython3 Makefile
make -j



# For Python3 Mac OSX : Need setup path to python3_dev include and g++ 
#brew install python3-dev
#brew install g++
# and setup PATH to include and lib
# Cellar is my username, you must change to your user
export PYTHON3_DEV_INCULE=/usr/local/Cellar/python@3.9/3.9.1_3/Frameworks/Python.framework/Versions/3.9/include
export PYTHON3_LIB_PATH=/usr/local/Cellar/python@3.9/3.9.1_3/Frameworks/Python.framework/Versions/3.9/lib
export PYTHON3_LIB_NAME=python3.9
export GPP_COMPILER=g++
cp -f MakefilePython3MACOS Makefile
make -j



```

- Hướng dẫn sử dụng
	+ Đặt file VietnameseTextNormalizer.so cùng thư mục chạy với file UnitTestVietnameseTextNormalizer.py
	+ Chạy file UnitTestVietnameseTextNormalizer.py


- Trong thư viện :
	+ Normalize : hàm chuẩn hóa dữ liệu Tiếng Việt cơ bản, chuẩn hóa i ngắn y dài đối với tên địa danh trong nước dựa theo văn bản hành chính nhà nước



- Note : 
	+ Nếu bạn truyền vào chuỗi/object ansi-utf8 thì hàm sẽ trả về ansi-utf8
	+ Nếu bạn truyền vào chuỗi/object unicode thì hàm sẽ trả về unicode
	+ Nếu bạn truyền vào 1 số nguyên hoặc loại object khác thì sẽ trả về luôn đối tượng bạn truyền vào


Sử dụng
------------

```python
# -*- coding: utf-8 -*-
import VietnameseTextNormalizer
a=VietnameseTextNormalizer.Normalize(u"UCS2 : Tôi làm việ ở ban công ngệ FPT, tôi là người viêt nam. hôm nay tôi ko thích ăn mì tôm. tôi làm đc 2 bài tập.");
print (a)


a=VietnameseTextNormalizer.Normalize(" UTF8 : Tôi làm việ ở ban công ngệ FPT, tôi là người viêt nam. hôm nay tôi ko thích ăn mì tôm. tôi làm đc 2 bài tập.");
print (a)

```

Tính năng
------------
Thư viện chuẩn hóa text Tiếng Việt cho python, có 1 số tính năng sau :
+ Chuẩn hóa dấu về kiểu phổ thông.
Ví dụ : [hoà] -> [hòa]

+ Chuẩn hóa các dấu dạng Combining Tone và các encode đặc biệt khác về dạng phổ thông. 
Ví dụ : 0x301 Combining Acute Accent

+ Xóa các kí tự đặc biệt của HTML còn sót lại . 
Ví dụ : dấu cách đặc biệt 0x200B Zero width space 

+ Điền nốt kí tự và dấu còn thiếu nếu chắc chắn. 
Ví dụ : 
tôi làm việ ở ban công ngệ FPT, tôi là người viêt nam 
-> tôi làm việc ở ban công nghệ FPT, tôi là người việt nam

+ Viết hoa 1 số tên địa danh phổ thông ở Việt Nam

+ Sửa lại việc viết sai chính tả ở 1 số vùng miền như : sự việc xẩy ra (Miền Bắc), xe ô tô bổn chỗ (Miền Trung), ...

+ Không làm lỗi cú pháp có sẵn của text 

+ Tự động sửa 1 số lỗi khác............

+ Code viết bằng C++, Wraper lại cho python, chạy rất nhanh. 3MB text Utf-8 chỉ cần 0.01s để xử lý (không tính I/O)


Note : Tiêu chí sửa của mình là chỉ sửa khi chắc chắn.  Vì thế nên không thể cover được tất cả các trường hợp nhưng mình sẽ update dần dần.

