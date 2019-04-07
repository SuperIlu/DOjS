
(* This AppleScript will fix the type and creators of source and project files for CodeWarrior. *)

property codewarrior_creator : "CWIE"
property codewarrior_project : "MMPr"

property folderKind : "folder"

on run
	tell application "Finder"
		try
			set folderName to (choose folder with prompt "Where are are the files to fix ?") as text
		on error -- e.g. user cancelled
			return
		end try
	end tell
	open (folderName)
end run

on open what
	tell application "Finder"
		if last character of (what as text) is ":" then --it's a folder
			-- get kind, for international systems
			set folderKind to the kind of folder what
			FixFolder(what as text) of me
		else
			display dialog ("Could not open " & what) buttons "OK" with icon 0 default button 1
		end if
	end tell
end open

on FixFolder(folder_name)
	tell application "Finder"
		-- walk the folder looking for support files 
		set folder_items to list folder folder_name without invisibles
		repeat with current_item in folder_items
			if (the kind of item (folder_name & current_item) is equal to folderKind) then
				-- recurse through subfolder
				FixFolder(folder_name & current_item & ":") of me
			else -- it's a file
				FixTypeAndCreator(folder_name & current_item) of me
			end if
		end repeat
	end tell
end FixFolder

on FixTypeAndCreator(file_name)
	tell application "Finder"
		if (file_name ends with ".c" or file_name ends with ".h" or Â
			file_name ends with ".r" or file_name ends with ".exp") then
			set file type of file file_name to "TEXT"
			set creator type of file file_name to codewarrior_creator
		else if file_name ends with ".mcp" then
			set file type of file file_name to codewarrior_project
			set creator type of file file_name to codewarrior_creator
		end if
	end tell
end FixTypeAndCreator
