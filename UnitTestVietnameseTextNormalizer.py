# -*- coding: utf-8 -*-
import VietnameseTextNormalizer

a=VietnameseTextNormalizer.Normalize(u"UCS2 : Tôi làm việ ở ban công ngệ FPT, tôi là người viêt nam. hôm nay tôi ko thích ăn mì tôm. tôi làm đc 2 bài tập.");
print (a)
print (type(a))


b=VietnameseTextNormalizer.Normalize(" UTF8 : Tôi làm việ ở ban công ngệ FPT, tôi là người viêt nam. hôm nay tôi ko thích ăn mì tôm. tôi làm đc 2 bài tập.");
print (b)
print (type(b))


#   print ("Kì vọng  bằng: '1', huyền: '2', ngã: '3', hỏi: '4', sắc: '5', nặng: '6' ")
#   print ("bằng  : " + VietnameseTextNormalizer.GetFirstTone("tôi"))
#   print ("huyền : " + VietnameseTextNormalizer.GetFirstTone("huyền"))
#   print ("ngã   : " + VietnameseTextNormalizer.GetFirstTone("ngã"))
#   print ("hỏi   : " + VietnameseTextNormalizer.GetFirstTone("hỏi"))
#   print ("sắc   : " + VietnameseTextNormalizer.GetFirstTone("sắc"))
#   print ("nặng  : " + VietnameseTextNormalizer.GetFirstTone("nặng"))
#   print ("Other : " + VietnameseTextNormalizer.GetFirstTone(''))
#   print ("Other : " + VietnameseTextNormalizer.GetFirstTone(u''))
#      
#   
#   a=VietnameseTextNormalizer.GetIBase(u"UCS2 : Tôi làm việ ở ban công ngệ FPT, tôi là người viêt nam. hôm nay tôi ko thích ăn mì tôm. tôi làm đc 2 bài tập.");
#   print (a)
#   print (type(a))
#   
#   
#   b=VietnameseTextNormalizer.GetIBase(" UTF8 : Tôi làm việ ở ban công ngệ FPT, tôi là người viêt nam. hôm nay tôi ko thích ăn mì tôm. tôi làm đc 2 bài tập.");
#   print (b)
#   print (type(b))