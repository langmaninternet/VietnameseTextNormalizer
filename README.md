## Thư viện chuẩn hóa text Tiếng Việt (Có sẵn wrapper cho Python2 và Python3)



Installation
------------

- Build
```sh
make -j
```

- Hướng dẫn sử dụng
	+ Đặt file VietnameseTextNormalizer.so cùng thư mục chạy với file UnitTestVietnameseTextNormalizer.py
	+ Chạy file UnitTestVietnameseTextNormalizer.py


- Trong thư viện có 3 hàm :
	+ Normalize : hàm chuẩn hóa dữ liệu Tiếng Việt cơ bản, chỉ chuẩn hóa i ngắn y dài đối với tên địa danh trong nước dựa theo văn bản hành chính nhà nước
	+ ASRNormalize : hàm chuẩn hóa dữ liệu cho ASR, khác với hàm cơ bản là chọn lựa i ngắn y dài rõ ràng theo độ phổ thông của từ. Ví dụ sẽ chọn [bệnh lý] thay cho [bệnh lí]
	+ ASRYToI : hàm để xử lý dữ liệu cho ASR, thống nhất hết về i ngắn 


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

+ Chuẩn hóa i ngắn y dài đối với tên các địa danh trong nước. Có mức chuẩn hóa cho ASR theo độ phổ thông của từ và perplexity của câu

+ Viết hoa 1 số tên địa danh phổ thông ở Việt Nam

+ Không làm lỗi cú pháp có sẵn của text 

+ Tự động sửa 1 số lỗi khác............

+ Code viết bằng C++, Wraper lại cho à python, chạy rất nhanh. 3MB text Utf-8 chỉ cần 0.01s để xử lý (không tính I/O)


Note : Tiêu chí sửa của mình là chỉ sửa khi chắc chắn.  Vì thế nên không thể cover được tất cả các trường hợp nhưng mình sẽ update dần dần.

