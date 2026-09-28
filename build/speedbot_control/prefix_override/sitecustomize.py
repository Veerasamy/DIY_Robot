import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/Arnab/DIY_AntiGravity/install/speedbot_control'
