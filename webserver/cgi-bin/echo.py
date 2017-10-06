#!/usr/bin/python3

import os
import sys

print(os.environ["QUERY_STRING"])
print(os.environ["REQUEST_METHOD"])
print(os.environ["CONTENT_TYPE"])
print(os.environ["CONTENT_LENGTH"])

print(os.environ["body"])
