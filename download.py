#!/usr/bin/python
#coding:utf-8

import sys,urllib
import os.path
import re
def download(url):
    img=urllib.urlopen(url)
    if img:
        localfile=open(os.path.basename(url),'wb')
        localfile.write(img.read())
        img.close()
        localfile.close()
    

if __name__ =='__main__':
    resource=urllib.urlopen(sys.argv[1])
    source = ''
    for i in resource:
        source += i
    obj=re.compile(r'(http:[^"]*(jpg|png|PNG|JPG))')
    elements=obj.findall(source)
    print elements
    for n in elements:
        download(n[0])
