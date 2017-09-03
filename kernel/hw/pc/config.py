config["ENABLE_FRAMEBUFFER"] = get_yes_no("Use VESA Framebuffer?", True)
if config["ENABLE_FRAMEBUFFER"]:
    config["ENABLE_FRAMEBUFFER_UNICODE"] = get_yes_no("Enable full Unicode BMP font (Adds around 1MB to binary)", False)
if config["ENABLE_FRAMEBUFFER"]:
    add_driver(True, "framebuffer")
    add_driver(False, "vesafb")
else:
    add_driver(False, "cgaterm")
add_driver(False, "8259")
add_driver(False, "idt")
add_driver(False, "pmm")

