# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-04-21
import argparse
import sys
import os
#from inspect import getmembers, isfunction
import inspect

from argparse import RawTextHelpFormatter

usage="""galaxy case runner
"""
epilog="""
bug reports
"""

BACK_GROUND_BLACK = '\033[1;40m'
BACK_GROUND_RED = '\033[1;41m'
BACK_GROUND_GREEN = '\033[1;42m'
BACK_GROUND_YELLOW = '\033[1;43m'
BACK_GROUND_BLUE = '\033[1;44m'
BACK_GROUND_BLUE_UNDERLINE = '\033[4;44m'
BACK_GROUND_PURPLE = '\033[1;45m'
BACK_GROUND_CYAN = '\033[1;46m'
BACK_GROUND_WHITE = '\033[1;47m'
TEXT_COLOR_BLACK = '\033[1;30m'
TEXT_COLOR_RED = '\033[1;31m'
TEXT_COLOR_GREEN = '\033[1;32m'
TEXT_COLOR_YELLOW = '\033[1;33m'
TEXT_COLOR_BLUE = '\033[1;34m'
TEXT_COLOR_PURPLE = '\033[1;35m'
TEXT_COLOR_CYAN = '\033[1;36m'
TEXT_COLOR_WHITE = '\033[1;37m'
RESET = '\033[1;m'
USE_SHELL = sys.platform.startswith("win")

class RichText(object):
    @classmethod
    def render(cls, text, bg_color, color=TEXT_COLOR_WHITE):
        """render
        """
        if USE_SHELL:
            return text
        result = []
        result.append(bg_color)
        result.append(color)
        result.append(text)
        result.append(RESET)
        return ''.join(result)

    @classmethod
    def render_text_color(cls, text, color):
        """render text with color
        """
        if USE_SHELL:
            return text
        return ''.join([color, text, RESET])

    @classmethod
    def render_green_text(cls, text):
        """render green text
        """
        return cls.render_text_color(text, TEXT_COLOR_GREEN)

    @classmethod
    def render_red_text(cls, text):
        """render red text
        """
        return cls.render_text_color(text, TEXT_COLOR_RED)

    @classmethod
    def render_blue_text(cls, text):
        """render blue text
        """
        return cls.render_text_color(text, TEXT_COLOR_BLUE)

    @classmethod
    def render_yellow_text(cls, text):
        """render yellow text
        """
        return cls.render_text_color(text, TEXT_COLOR_YELLOW)



def build_parser():
    parser = argparse.ArgumentParser(
                       prog="case_runner.py",
                       formatter_class=RawTextHelpFormatter,
                       epilog=epilog,
                       usage=usage)
    parser.add_argument("-d",
                        default=None,
                        dest="case_dir",
                        help="specify case folder")
    return parser

def find_cases(case_dir):
    """
    TODO,µ›πÈ≤È’“
    """
    ret = []
    for file in os.listdir(case_dir):
        if not os.path.isfile(os.sep.join([case_dir,file])):
            continue
        if file.startswith("case") and file.endswith(".py"):
            ret.append(file)
    return ret

def run_a_case(case_dir,case_file):
    current_path = os.getcwd()
    statics = {}
    no_exception = True
    try:
        os.chdir(case_dir)
        print "start to run %s"%RichText.render_green_text(case_file)
        print "======================output============================"
        module_name = case_file.replace(".py","")
        module = __import__(module_name)
        set_up_func = None
        clean_func = None
        test_func_dict={}
        for o in inspect.getmembers(module):
            if not inspect.isfunction(o[1]):
                continue
            if o[0]=="set_up":
                set_up_func = o[1]
                continue
            if o[0]=="clean":
                clean_func=o[1]
            if o[0].find("test") != -1:
                test_func_dict[o[0]] = o[1]
        if set_up_func:
            try:
                set_up_func()
                print "set_up %s"%RichText.render_green_text("ok")
            except Exception as ex:
                print "set_up %s %s"%(RichText.render_red_text("error"),str(ex))

        for key in test_func_dict:
            try:
                test_func_dict[key]()
                print "%s %s"%(key,RichText.render_green_text("ok"))
                statics[key]=(True,"")
            except Exception as e:
                print "%s %s %s"%(key,RichText.render_red_text("error"),str(e))
                statics[key]=(False,str(e))
                no_exception=False
        if clean_func:
            try:
                clean_func()
                print "clean %s"%RichText.render_green_text("ok")
            except Excetpion as e:
                print "clean %s %s"(RichText.render_red_text("error"),str(e))
        print "======================end=============================="
    finally:
        os.chdir(current_path)
    return no_exception,statics

def main(options):
    if not options.case_dir:
        print "%s is required"%RichText.render_red_text("-d")
        sys.exit(-1)
    case_dir = options.case_dir
    if not os.path.exists(case_dir):
        print "%s does not exist"%RictText.render_red_text(case_dir)
        sys.exit(-1)
    case_list = find_cases(case_dir)
    case_statics_dict = {}
    for case in case_list:
        status,statics = run_a_case(case_dir,case)
        case_statics_dict[case] = (status,statics)
    print "======================case statics============================="
    has_error = False
    for key in case_statics_dict:
        case_statics = case_statics_dict[key]
        if case_statics[0]:
            print "run %s successully"%RichText.render_green_text(key)
        else:
            print "run %s with error"%RichText.render_red_text(key)
            has_error = True
    if has_error:
        sys.exit(-2)

if __name__ == "__main__":
    parser = build_parser()
    options = parser.parse_args()
    main(options)
