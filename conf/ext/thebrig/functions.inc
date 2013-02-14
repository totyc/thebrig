<?php

function thebrig_unregister() {
 	global $config;
 	
 	$i = 0;
 	if (is_array($config['rc']['postinit']['cmd'])) {
 		for ($i; $i < count($config['rc']['postinit']['cmd']); $i++) {
 			if (preg_match('/thebrig_start\.sh/', $config['rc']['postinit']['cmd'][$i]))
 				unset($config['rc']['postinit']['cmd'][$i]);	// Disable the old startup
 		}	// end for
 	} // end if (is array)

 	// remove TheBrig related config and webgui settings - store folder location temporarily
 	$brigroot = $config['thebrig']['rootfolder'];
 	// clears the config variables
 	unset( $config['thebrig'] );
 	write_config();
 	
 	// remove all the webgui stuff (particularly useful for "full" installs
 	
 	// First, glob all the php files from the extensions folder into an array of fully pathed filenames
 	// and work through them one at a time as file. 
 	foreach ( glob( "{$brigroot}/conf/ext/thebrig/*.php" ) as $file ) {
 			// Carry out the string replacement operation. For each file, search for the string 
 			//"{$brigroot}/ext/thebrig" and replace it with /usr/local/www. This will create a 
 			// string that has a fully pathed filename for the extension's webpages (which are 
 			// actually symbolic links we need to destroy.
 		$file = str_replace("{$brigroot}/conf/ext/thebrig", "/usr/local/www", $file);
 			// only carry out the delete if it is a symbolic link (in case users have customised their own)
 		if ( is_link( $file ) ) {
 			unlink( $file ); 	// remove the ymbolic link
 		} // end if
 	} // end for loop
 	
 	// This loops runs for each of the files in the extension folder for TheBrig, and removes
 	// all the files
 	foreach ( glob( "/usr/local/www/ext/thebrig/*" ) as $file ) {
 		unlink( $file );
 	}

 	// Check to see if the folder is a folder, then remove it
 	if ( is_dir( "/usr/local/www/ext/thebrig" ) ) {
 		rmdir( "/usr/local/www/ext/thebrig" );
 	}
	// Make the former root folder writeable by all, so that it can be easily deleted.
 	mwexec("/bin/chmod -R 777 {$brigroot}");
} // end of thebrig_unregister function

// Function name: 	thebrig_populate
// Inputs: 			none
// Outputs: 		logical result of the creation of the proper directory structure in the install folder
// Description:		This function allows the installation (or movement of) the installation folder and
//					directory tree
function thebrig_populate ($new_location, $old_location) {
	global $config ;
	// First we need to see if old location has a directory tree. If it does, we chould clean up after
	// ourselves. This involves moving the php and .inc files. Then, the creation of the directory structure
	// and startup scripts can progress normally
	if ( is_dir ( $old_location )) {
		// This indicates that the folders are actually different
		if ( strcmp ( $old_location , $new_location ) ){
			mwexec("/bin/mv {$old_location}/conf {$new_location}/");	// moves the conf
			mwexec( "/bin/rm -r {$old_location}");
		}	// removes the old directory }

		// This section finds and unsets the startup command for the extension (code from slimserver install)
		$i = 0;
		if (is_array($config['rc']['postinit']['cmd'])) {
			for ($i; $i < count($config['rc']['postinit']['cmd']); $i++) {
				if (preg_match('/thebrig_start\.sh/', $config['rc']['postinit']['cmd'][$i]))
					unset($config['rc']['postinit']['cmd'][$i]);	// Disable the old startup
			 	}	// end for		
		} // end if (is array)
	} // end if

	mwexec( "/bin/mkdir -p {$new_location}/work");
	mwexec( "/usr/bin/touch -p ${new_location}/conf/jail_start");

	// This will search the startup commands to find the appropriate index to place thebrig's startup script command
	$i = 0;
	// Don't want to attempt array operations if there are no postinit commands.
	if ( is_array( $config['rc']['postinit']['cmd'] ) ) {
		for ($i; $i < count($config['rc']['postinit']['cmd']); $i++) {
			// This loops through all the post init command script entries, and if it finds thebrig_start.sh (placed by hand), 
			// it will update the entry to reflect the new location by breaking out of the for loop at the correct index.
			if ( preg_match('/thebrig_start\.sh/', $config['rc']['postinit']['cmd'][$i]))
				break;
		} // end of for loop
	} // end of array if statment
	// update the value of the postinit command.
	$config['rc']['postinit']['cmd'][$i] = "/bin/sh {$new_location}/conf/bin/thebrig_start.sh";
	// update all the symbolic links and such by executing the startup script from the new location
	//mwexec("/bin/sh {$new_location}/conf/bin/thebrig_start.sh");
}	// end of thebrig_populate()

// Function name: 	check_thebrig
// Inputs: 			none
// Outputs: 		logical result if thebrig has been installed
// Description:		This function evaluates whether or not thebrig has been properly installed
//					to the root folder yet
function check_thebrig() {
	global $config;		// import global config
		// The presence of /conf/jail_start is an analogue for the installation
	return file_exists($config['thebrig']['rootfolder'] . "/conf/jail_start");
}

// Function name: 	thebrig_radio_list
// Inputs: 			file_list			array of filenames with suffix to create list for
// Outputs: 		installFiles		html formatted block with a radio next to each file
// Description:		This function creates an html code block with the files listed on the right
//					and radio buttons next to each on the left. 
function thebrig_radio_list ($file_list) {
	global $config ;		// import the global config array 
	$installFiles = "";		// Initialize installFiles as an empty string so we can concatenate in the for loop
	if (is_dir($config['thebrig']['rootfolder'])) {		// check if the folder is a directory, so it doesn't choke
		foreach ( $file_list as $file) {
			$file = str_replace($config['thebrig']['rootfolder'] . "/", "", $file);
			$installFiles .= "<input type=\"radio\" name=\"installfile\" value=\"$file\"> "
			. str_replace("work/", "", $file)
			. "<br/>";
			} // end of completed folder, filename, suffix creation
	} // end of verifying rootfolder as valid location
	return $installFiles ;
}

// Function name: 	thebrig_checkbox_list
// Inputs: 			file_list			array of filenames with suffix to create list for
// Outputs: 		installFiles		html formatted block with a checkbox next to each file
// Description:		This function creates an html code block with the files listed on the right
//					and checkboxes next to each on the left.
function thebrig_checkbox_list ($file_list) {
	global $config ;
	$installFiles = "";
	if ( is_dir($config['thebrig']['rootfolder']) && count( $file_list ) > 0 ) {
		// Since we will be moving through the entirety of the $file_list array, we need a separate index counter, $i, 
		// which will start at 0. 
		$i = 0;
		// We will now figure out how many files we have. This number divided by 3 and rounded down
		// will tell us how many complete (two entry) rows in the table. Thus, we will run one loop to take care of each
		// complete row.
		$installFiles .= "<table border=\"0\" cellpadding=\"1\" width=\"100%\">";
		for ( $k = 1 ; $k <= floor( ( count( $file_list ) ) / 3 ) ; $k++ ) {
			$installFiles .= "<tr>";
				// This code removes the pesky path data from the globbed file list, then creates
				// an html checkbox entry for the file, and increments to the next index (i) 
				// This code is repeated for the other two columns.
				// Left Column
			$installFiles .= "<td width=\"33%\">" ;
			$file = str_replace( "{$config['thebrig']['rootfolder']}/work/" , "", $file_list[$i]);
			$installFiles .= "<input type=\"checkbox\" name=\"formFiles[]\" value={$file}> "
			. " {$file} ";
			$i++;
			$installFiles .= "</td>";
			// Middle Column
			$installFiles .= "<td width=\"33%\">" ;
			$file = str_replace( "{$config['thebrig']['rootfolder']}/work/" , "", $file_list[$i]);
			$installFiles .= "<input type=\"checkbox\" name=\"formFiles[]\" value={$file}> "
			.  " {$file} ";
			$i++;
			// Right Column - with a minor change - it is the end of a row, so we have a new line
			$installFiles .= "<td width=\"33%\">" ;
			$file = str_replace ("{$config['thebrig']['rootfolder']}/work/" , "", $file_list[$i]);
			$installFiles .= "<input type=\"checkbox\" name=\"formFiles[]\" value={$file}> "
			.  " {$file}";
			$i++ ; 
			$installFiles .= "</td>";
			$installFiles .= "</tr>";
		} // end of row creation
		// This if statement checks if the remainder of the total list of files divided by 3 is at least 1, meaning
		// there are one or two entries in the bottom, incomplete row.
		if ( count( $file_list ) % 3 >= 1 ) {
			// This code removes the pesky path data from the globbed file list, then creates
			// an html checkbox entry for the file, and increments to the next index (i)
			$installFiles .= "<tr>";
			$installFiles .= "<td width=\"33%\">" ;
			$file = str_replace ("{$config['thebrig']['rootfolder']}/work/" , "", $file_list[$i]);
			$installFiles .= "<input type=\"checkbox\" name=\"formFiles[]\" value={$file}> "
			.  " {$file} ";
			$i++ ;
			$installFiles .= "</td>";
			// This if statement checks if the remainder of the total list of files divided by 3 is 2, meaning
			// there are two entries in the bottom, incomplete row.
			if ( count( $file_list ) % 3 == 2 ) {
				// This code removes the pesky path data from the globbed file list, then creates
				// an html checkbox entry for the file, and increments to the next index (i)
				$installFiles .= "<td width=\"33%\">" ;
				$file = str_replace ("{$config['thebrig']['rootfolder']}/work/" , "", $file_list[$i]);
				$installFiles .= "<input type=\"checkbox\" name=\"formFiles[]\" value={$file}> "
				.  " {$file}";
				$i++ ;
				$installFiles .= "</td>";
			} // end if filecount % 3 = 2
			// There is only one entry in the bottom row, so we need to finish the line
			else {
				$installFiles .= "</tr>";
			} // end else
		} // end if filecount % 3 >= 1
		// End the table
		$installFiles .= "</table>" ;
	} // end if root folder is a valid directory & there are at least some files
	return $installFiles ;
}

// Function name: 	thebrig_tarball_list
// Inputs: 			file_list			array of filenames with suffix to create list for
//					exclude				Optional array used to remove certain results
// Outputs: 		file_list			html formatted block with a radio next to each file
// Description:		This function creates an html code block with the files listed on the right
//					and radio buttons next to each on the left.
function thebrig_tarball_list ($contains , $exclude='') {
	global $config ;
	// This function creates a list of tarballs that match a certain filename pattern
	$installFiles = "";
	if ( is_dir( $config['thebrig']['rootfolder'] )) {
		$raw_list = glob("{$config['thebrig']['rootfolder']}/work/{$contains}.{txz,tgz}", GLOB_BRACE);
		$file_list = array_unique( $raw_list );
		if ( $exclude ) {
			foreach ( $exclude as $search_pattern ) {
				$file_list = preg_grep( "/{$search_pattern}/" , $file_list , PREG_GREP_INVERT );
			}
		}
	} // end of verifying rootfolder as valid location
	return $file_list ;
}

// Function name: 	thebrig_dl_list
// Inputs: 			file_list			array of filenames with suffix to create list for, with each filename
//										consisting of _partial_{filesize}.
// Outputs: 		partFiles			html formatted block with a status bar next to each file
// Description:		This function creates an html code block with the each of the files listed in file_list
//					and then a progress bar indicating how much it has downloaded. This is accomplished when
// 					the fetch operation first determines the file's size, and appends it as part of the file's name.
//					Thus, the data needed to determine download progress is already with the file.
function thebrig_dl_list ( $file_list ) {
global $config ;
	$partFiles = "";
	if ( is_dir($config['thebrig']['rootfolder']) && count( $file_list ) > 0 ) {
		// We will now figure out how many files we have. This number divided by 3 and rounded down
		// will tell us how many complete (two entry) rows in the table. Thus, we will run one loop to take care of each
		// complete row.
		$partFiles .= "";
		for ( $k = 0 ; $k < ( count( $file_list ) ) ; $k++ ) {
			// This code removes the pesky path data from the globbed file list, as well as removing the "partial_{filesize}"
			// It also converts all the underscores to spaces. This cleaned filename is then placed in the cell and the cell closed
			$cleanfile = str_replace( "{$config['thebrig']['rootfolder']}/work/" , "", $file_list[$k]);
			// We find the index of the start of "_partial_". This is used to figure out where the size is.
			$partIndex = strripos($cleanfile , "_partial_");
			// We can now infer where the {filesize} starts, by adding 9 to the partIndex (length of _partial_)
			$sizeIndex = $partIndex + 9;
			// We also find the index of the start of the file suffix, to figure out where the end of the size lives.
			$suffixIndex = strripos($cleanfile , ".");
			// Now we can use where the number starts and the suffix begins to determine how many characters the filesize
			$sizeLength = $suffixIndex - $sizeIndex;
			// Now we extract the string starting at the start of the size, and going for as many characters are the filesize
			// string is. Then, we convert this string of numbers to an integer.
			$targetSize = intval ( substr( $cleanfile , $sizeIndex , $sizeLength ) ) ;
			// We also need to convert the filesizes to MiB for subsequent display
			$nicetargetSize = round( $targetSize / 1048576 , 2);
			// Obtain the current filesize from the OS
			$currentSize = filesize( $file_list[$k] );
			
			// Begin the first row, and first cell
			$partFiles .= "<tr><td width=\"25%\" class=\"vncellt\">";
			// Chop off everything after the first _ of _partial_ in the filename
			$cleanfile = substr( $cleanfile , 0 , $partIndex ) ;
			// Replace the hyphen with a space
			$cleanfile = str_replace( "-" , " ", $cleanfile ) ;
			// end the cell
			$partFiles .= "{$cleanfile}</td>";

			// Now we start building the right column, the one witht the progress bar
			$partFiles .= "<td width=\"75%\" class=\"listr\">" ;
			$pct_done = floor( $currentSize * 100 / $targetSize ) ;
			$pct_rem = 100 - $pct_done;
			
			$partFiles .= "<img src=\"bar_left.gif\" class=\"progbarl\" alt>";
			$partFiles .= "<img src=\"bar_blue.gif\" name=\"file{$k}u\" id=\"file{$k}u\" width=\"{$pct_done}px\" class=\"progbarcf\" alt>";
			$partFiles .= "<img src=\"bar_gray.gif\" name=\"file{$k}f\" id=\"file{$k}f\" width=\"{$pct_rem}px\" class=\"progbarc\" alt>";
			$partFiles .= "<img src=\"bar_right.gif\" class=\"progbarr\" alt>";
			$partFiles .= "<input style=\"padding: 0; border: 0;\" size=\"30\" name=\"file{$k}\" id=\"file{$k}\" value=\"{$pct_done}% of {$nicetargetSize}MiB\">";
			//$partFiles .= $pct_done ;
			
			$partFiles .= "</td>" ;
			$partFiles .= "</tr>";
		} // end of row creation
	} // end if root folder is a valid directory & there are at least some files
	return $partFiles ;
}

// Function name: 	thebrig_menu_list
// Inputs: 			list				array of filenames with suffix to create list for
// Outputs: 		menu				html formatted block of the menu
// Description:		This function creates an html code block with a dropdown box
function thebrig_menu_list ( $list , $list_name , $chosen ) {
	global $config ;
	// This function
	$menu = "<select name =\"{$list_name}\" id=\"{$list_name}_menu\">";
	// Build the select box one list item at a time
	foreach ( $list as $element) {
		// Check if the currently inspected element of the array 
		if ( strcmp($element, $chosen) == 0 ) {
			$menu .= "<option selected value = \"$element\"> $element </option> " ;
		}
		else {
			$menu .= "<option value = \"$element\"> $element </option> " ;
		}
	} // end of completed folder, filename, suffix creation
	$menu .= "</select>";
	return $menu ;
}


function get_nice_filesize ($file) {
	$dsize = filesize($file);
	if (strlen($dsize) <= 9 && strlen($dsize) >= 7) {
		$dsize = number_format($dsize / 1048576,1);
		return "$dsize MB";
	}
	elseif (strlen($dsize) >= 10) {
		$dsize = number_format($dsize / 1073741824,1);
		return "$dsize GB";
	}
	else {
		$dsize = number_format($dsize / 1024,1);
		return "$dsize KB";
	}
}