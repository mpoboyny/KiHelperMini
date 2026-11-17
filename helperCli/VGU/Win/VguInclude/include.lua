--
--This file is part of VirtualGUser project.
--Copyright ©2017 Mykhajlo Pobojnyj <mpoboyny@googlemail.com>

--This program is free software; you can redistribute it and/or
--modify it under the terms of the GNU General Public License
--as published by the Free Software Foundation; version 2.1 (LGPL-2.1) or later of the License.

--This program is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--GNU General Public License for more details.

--You should have received a copy of the GNU General Public License
--along with this program; if not, write to the Free Software
--Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
--
-- include.lua

function IsWindowParent(wnd, parentWnd)
	local allParents = wnd:getAllParent()
	local wndId = parentWnd:getId()
	for i in allParents do
		if wndId == allParents[i]:getId() then
			return true
		end
	end
	return false
end

function ShowAllParent(wnd)
	print("\n------- ShowAllParent --------\n")
	print("Window id: ", wnd:getId())
	print("Window title: ", wnd:getTitle())
	local allParents = wnd:getAllParent()
	print("Parents count: ", table.getn(allParents))
	for i in allParents do
		print(i, ": ")
		print("\tWindow id: ", allParents[i]:getId())
		print("\tWindow title: ", allParents[i]:getTitle())
        print("\tWindow process id: ", allParents[i]:getProcess():getPrcId())
	end
end

function ShowAllChild(wnd)
	print("\n------- ShowAllChild --------\n")
	print("Window id: ", wnd:getId())
	print("Window title: ", wnd:getTitle())
	local allChilds = wnd:getAllChild()
	print("Child count: ", table.getn(allChilds))
	for i in allChilds do
		print(i, ": ")
		print("\tWindow id: ", allChilds[i]:getId())
		print("\tWindow title: ", allChilds[i]:getTitle())
		print("\tWindow text: ", allChilds[i]:getText())
		print("\tWindow class: ", allChilds[i]:getClassName())
	end
end

function ShowWndGeom(wnd)
	io.write("\n------- ShowWndGeom --------\n")
	if wnd and wnd:getId() > 0 then
		local rc = wnd:getGeometry()
		io.write("posX: ", rc["posX"])
		io.write(" posY: ", rc["posY"])
		io.write(" width: ", rc["width"])
		io.write(" height: ", rc["height"], "\n")
		-- for i in rc do
			-- io.write(i, ": ", rc[i], " ")
		-- end
	end
end

function ShowAllWndProp(wnd)
	print("\n------- ShowWndProp --------\n")
	print("Table count: ", table.getn(wnd))
	for i in wnd do
		if wnd[i] and wnd[i]:getId() > 0 then
			print("Id: ", wnd[i]:getId(wnd[i]))
			print("Title: ", wnd[i]:getTitle())
			print("Class: ", wnd[i]:getClassName())
            print("Process id: ", wnd[i]:getProcess():getPrcId())
		else
			print("Window with id: ", wnd[i]:getId(), " is not valide or somethings went wrong ...")
		end
	end
end

function SleepSec(sec, msg)
	local msgSleep = "\nThe runtime go sleep for "..sec.." seconds ... "
	if msg then
		msgSleep = msgSleep..msg
	end
	print(msgSleep)
	MPRuntime.sleep(sec)
end

function KillAllProcess(prc)
	print("\n------- KillAllProcess --------\n")
	SleepSec(3)
	print("Table count: ", table.getn(prc))
	for i in prc do
		if prc[i] and prc[i]:isPrcRunning() then
			print("The process with pid: ", prc[i]:getPrcId(), " is running.\nTerminate ...")
			if prc[i]:terminatePrc() then
				print("\tdone.")
			else
				print("\tstill running !")
			end
		else
			print("The process with pid: ", prc[i]:getPrcId(), " is not running or somethings went wrong ...")
		end
	end
end

function StartCheck()
	print("\n------- StartCheck --------")
	print("At: "..os.date('%H:%M:%S %d.%m.%Y').."\n")
	local argCount = 0
	print("My application name: ", MPRuntime.getAppName())
	print("Arguments:")
	local allArg = MPRuntime.getAppArgv()
	for arg in allArg do
		print("\t", arg, ": ", allArg[arg])
	end
	argCount = table.getn(allArg)
	return argCount, allArg
end

function GetLastChildFromPointStd(wnd, x, y)
	local allCurrChilds = MPWindow.getAllChildFromPoint(wnd, x, y)
	return allCurrChilds[table.getn(allCurrChilds)]
end

function SaveScreenShot(wnd, filePath)
	if wnd:saveScreenShot(filePath) then
		print("\nSave screen shot to ", filePath, " - OK")
	else
		print("\nSave screen shot to ", filePath, " - FAILED")
	end
end

function CompareToFile(wnd, filePath)
	print("\n", wnd:cmpScreenShotWithBmp(filePath), "% of screen shot are equal with: ", filePath)
end

function SaveScreenShotEx(wnd, filePath, x, y, width, height)
	if wnd:saveScreenShotEx(filePath, x, y, width, height) then
		print("\nSave screen shot to ", filePath, " - OK")
	else
		print("\nSave screen shot to ", filePath, " - FAILED")
	end
end

function CompareToFileEx(wnd, filePath, x, y, width, height)
	print("\n", wnd:cmpScreenShotWithBmpEx(filePath, x, y, width, height), "% of screen shot are equal with: ", filePath)
end

local posXclickStd, posYclickStd

function MouseRecCallBack(posX, posY)
	io.write("\n------- mouseRecCallBack --------\n")
	io.write("mouseRecCallBack, posX: ", posX, " posY: ", posY, "\n")
	posXclickStd = posX
	posYclickStd = posY
end

function RecWindow(recFilesPrefix, wnd, activateWnd)
	local currOS = MPRuntime.getOStype()
	local recFile = io.open(recFilesPrefix..".txt", "w+")
	io.output(recFile)
	io.write("Click record for window: ",  wnd:getTitle(), "\n")
	print ("Run mouse record loop...")
	local activateWndEx = true
	if activateWnd ~= nil then
		activateWndEx = activateWnd
	end
	while wnd:startMouseRec(activateWndEx, MouseRecCallBack) do
        wnd:clickWindowExL(posXclickStd, posYclickStd)
		local wnd = MPWindow.getWindowFromPoint(posXclickStd, posYclickStd)
		if wnd and wnd:getId() > 0 then
			io.write("\tWindow id: ", wnd:getId())
			io.write("\tWindow title: ", wnd:getTitle())
			io.write("\tWindow text: ", wnd:getText())
			io.write("\tWindow class: ", wnd:getClassName())
			local rc = wnd:getGeometry()
			SaveScreenShot(wnd, recFilesPrefix.."_"..posXclickStd.."x"..posYclickStd..".bmp")
		end
	end
	io.close(recFile)
end

function GetWordSplit(str)
	local words = {}
	local i = 0
	for w in string.gfind(str, "%w+") do
		i = i +1
		words[i] = w
		-- print(i..": "..w)
	end
	return i, words 
end
