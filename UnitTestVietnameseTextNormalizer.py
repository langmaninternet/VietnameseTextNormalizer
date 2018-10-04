# -*- coding: utf-8 -*-
import VietnameseTextNormalizer
a=VietnameseTextNormalizer.Normalize(u"UCS2 : Tôi làm việ ở ban công ngệ FPT, tôi là người viêt nam. hôm nay tôi ko thích ăn mì tôm. tôi làm đc 2 bài tập.");
print (a)


a=VietnameseTextNormalizer.Normalize(" UTF8 : Tôi làm việ ở ban công ngệ FPT, tôi là người viêt nam. hôm nay tôi ko thích ăn mì tôm. tôi làm đc 2 bài tập.");
print (a)