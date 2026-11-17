require("include.lua")


local deskWnd = MPWindow.getDesktopWindow()
local recFilePrefix = "FindWindow"
print("All output files have prefix "..recFilePrefix)
RecWindow(recFilePrefix, deskWnd)
print("Done.\n")
