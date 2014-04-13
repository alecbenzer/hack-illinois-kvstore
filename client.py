from kvstore import *

if __name__=="__main__":
    do_del("myKey")
    do_get("myKey")
    do_set("myKey", "myVal")
    do_get("myKey")
    do_set("myKey", "newVal")
    do_get("myKey")
