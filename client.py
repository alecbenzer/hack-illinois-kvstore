from kvstore import do_get, do_set, send_message

if __name__=="__main__":
    do_set("myKey", "myVal")
    do_get("myKey")
