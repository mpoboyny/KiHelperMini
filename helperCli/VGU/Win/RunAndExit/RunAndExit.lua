--
-- RunAndExit.lua
--
currOS = MPRuntime.getOStype()
print("currOS: "..currOS)

includeFolder = os.getenv("VGU_INC")
print("VGU_INC: "..includeFolder)

require(includeFolder.."include.lua")


g_argc, g_argv = StartCheck()

local prc = MPProcess.createPrc(g_argv[3], "-m", string.gsub(g_argv[4], "\\", "\\\\"), true, true)
if not prc then
    print("MPProcess.createPrc failed")
	os.exit(111)
end

SleepSec(5, "Let start...")

local wnd = prc:getPrcMainWindow()

if not wnd then
    print("not wnd")
	os.exit(111)
end

wnd:printText("Hallo")
SleepSec(2)
wnd:keyType({'Return'})
