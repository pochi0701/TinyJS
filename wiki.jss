<?
/*  Wiki
 */
// setup some global storage
config = {"PAWFALIKI_VERSION":"0.1.1"}; // Wiki version
//================
//================
// CONFIGURATION
//================
//================
// GENERAL: General configuration stuff
config.GENERAL={
"TITLE"          :"NEON WIKI",				// Title of the wiki
"HOMEPAGE"       :"NEON WIKI",				// The title of the homepage
"ADMIN"          :"webmaster at nowhere dot example",	// not used currently
"CSS"            :"Wiki:wiki.css",			// CSS file (title:filename)
"PAGES_DIRECTORY":"./WikiPages/",			// Path to stored wiki pages
"TEMP_DIRECTORY" :"./WikiTemp/",			// Path to temporary directory for backups
"MODTIME_FORMAT" :"(%Y-%m-%d %H:%M:%S)",		// date() compatible format string for the pagelist
"SHOW_CONTROLS"  :true,					// show all the wiki controls - edit, save, PageList etc...
"DEBUG"          :false,				// display debug information (pagegen time, uptime, load)
"ENCODE"         :"utf-8"				// "shift-jis","utf-8"...
};

// SYNTAX: Wiki editing syntax
config.SYNTAX={
"SHOW_BOX"       :true,                                 // Display the wiki syntax box on edit page
"WIKIWORDS"      :false,                                // Auto-generation of links from WikiWords
"AUTOCREATE"     :true,                                 // Display ? next to wiki pages that don't exist yet.
"HTMLCODE"       :true                                  // Allows raw html using %% tags
};

// BACKUP: Backup & Restore settings
config.BACKUP={
"ENABLE"         :true,                                 // Enable backup & restore
"USE_ZLIB"       :true,                                 // If available use the libz module to produce gzipped backups
"MAX_SIZE"       :3000000                               // maximum file size (in bytes) for uploading restore files
};

// RSS: RSS feed
config.RSS={
"ENABLE"         :true,                                 // Enable rss support (http://mywiki.example?format=rss)
"ITEMS"          :10,                                   // The number of items to display in rss feed (-1 for all).
"TITLE_MODTIME"  : false,                               // Prints the modification time in the item title.
"MODTIME_FORMAT" :"(Y-m-d H:i:s T)"                     // date() compatible format string
};

// CHANGES: email page changes
config.EMAIL={
"ENABLE"         :false,                                // do we email page changes?
"CHANGES_TO"     :"admin@nowhere.example",              // if so, where to
"CHANGES_FROM"   :"changes@nowhere.example",            // & where from
"MODTIME_FORMAT" :"%Y-%m-%d %H:%M:%S",                        // date() compatible format string for the pagelist
"SHOW_IP"        :false                                 // show the modifiers ip in the email subject
};

// USERS: setup user passwords
config.USERS={
"admin2"         :"adminpassword",                      // changing this would be a good idea!
"group1"         :"group1password"                      // create a new user password
};

// RESTRICTED: give access to some users to edit restricted pages
config.RESTRICTED={
"RestoreWiki"    :["admin"]                             // only admin can restore wiki pages
};

// LOCALE: text for some titles, icons, etc - you can use wiki syntax in these for images etc...
config.LOCALE={
"EDIT_TITLE"     :"edit: ",                             // title prefix for edit pages
"HOMEPAGE_LINK"  :"[[HomePage]]",                       // link to the homepage
"PAGELIST_LINK"  :"[[PageList]]",                       // link to the pagelist
"REQ_PASSWORD"   :"(locked)",                           // printed next to the edit btn on a locked page
"PASSWORD_TEXT"  :"Password:"                           // printed next to the password entry box
};
// SPECIAL PAGES - reserved and unmodifiable by users
config.SPECIAL={
"PageList"       :1,                                    // the page list
"BackupWiki"     :1,                                    // the backup page
"RestoreWiki"    :1                                     // the restore page
};
// MISC: Misc stuff
config.MISC={
"EXTERNALLINKS_NEWWINDOW"      :false,                  // Open external links in a new window
"REQ_PASSWORD_TEXT_IN_EDIT_BTN":false                   // Include the req password text in the edit button
};
config.INTERNAL={"VERBATIM":[], "ERRORS":[], "DATA":[]};

//===========================================================================
//===========================================================================
// initialise our style sheets
function isset( object )
{
    return (!(object == undefined));
}
function css( pagename )
{
    //global config;
    css = config.GENERAL.CSS;
    if (css!="")
    {
        tokens = css.split(":");
        title = tokens[0];
        path = tokens[1];//.remove(1).join(":");
        print( "\t<link rel=\"stylesheet\" type=\"text/css\" href=\""+path+"\" title=\""+title+"\" />\n");
        if ( config.RSS.ENABLE && pagename=="HomePage" ){
            print( "\t<link rel=\"alternate\" title=\""+config.GENERAL.TITLE+
                    " RSS\" href=\""+_SERVER.SCRIPT_NAME+
                    "?format=rss\" type=\"application/rss+xml\" />\n" );
        }
    }
}

// emails page changes
function emailChanges( title, contents )
{
    if ( config.EMAIL.ENABLE )
    {
        date = date(config.EMAIL.MODTIME_FORMAT);
        subject = title+" :: "+date;
        if ( config.EMAIL.SHOW_IP )
        {
            ipaddress = _SERVER.REMOTE_ADDR;
            subject += " :: IP "+ipaddress;
        }
        mail( config.EMAIL.CHANGES_TO, subject, contents, "From: "+config.EMAIL.CHANGES_FROM+"\r\n" );
    }
}

// writes a file to disk
function writeFile( title, contents )
{
    if( FILE.saveToFile(pagePath(title),contents) == 0 ){
        error("Cannot write to server's file: "+pagePath( title ));
        return 2;
    }
    
    // email page changes
    emailChanges( title, contents );
    return 0;    
}

// reads the contents of a file into a string (php<4.3.0 friendly)
function wikiReadFile( filename )
{
    result = FILE.loadFromFile(filename);
    return result;
}

// returns the contents of a directory (php<4.3.0 friendly)
function wikiReadDir(path)
{
    return eval( FILE.scandir(path) );    
}

// init the wiki if no pages exist
function initWiki( title )
{
    contents = "Hello and welcome to Wiki!";    
    writeFile( title, contents );
}

// get the title of a page
function getTitle()
{
    page = "";
    if ( !isset(_GET.page) )
    {
        page = "HomePage";
        if ( !pageExists( page ) )
        {
            initWiki( page );
        }
    }
    else
    {
        page = _GET.page;
        if (page.split("/").length>1){
            page = "HomePage";
        }
    }
    return page;
}

// get the current wiki 'mode'
function getMode()
{
    mode = "";
    if ( !isset(_POST.mode) )
    {
        mode = "display";
    }
    else
    {
        mode = _POST.mode;
    }
    return mode;
}

// check 
function authPassword( title, password )
{
    auth = false;
    for( i = 0 ; i < config.RESTRICTED[title].length ; i++ ){
        user = config.RESTRICTED[title][i];
        if (config.USERS[user]==_POST.password) 
        auth = true;
    }
    return auth;
}

// update the wiki - save/edit/backup/restore/cancel
function updateWiki( modes, title, config )
{    
    contents = "";
    backupEnabled = config.BACKUP.ENABLE;
    // cleanup any temp files
    if ( backupEnabled ){
        cleanupTempFiles();
    }
    // backup the wiki
    if ( title=="BackupWiki" ){
        if( backupEnabled )
        {
            wikiname = config.GENERAL.TITLE;
            wikiname.replace( " ", "_" );
            //date = date( "Y-m-d_H-i-s" );
            //filename = tempDir()+wikiname+"_"+date+"+bkup";
            filename = tempDir()+wikiname+"_bkup";
            backupPages( filename );
            modes.mode = "backupwiki";
        }
        else
        {
            error( "Backups have been disabled+" );
        }
    }
    // restore from backup
    if ( title=="RestoreWiki" )
        if ( backupEnabled )
        {
            if ( modes.mode=="restorewiki" && isset(_FILES.userfile.name) )
                restorePages();
            else
                modes.mode = "restorewiki";
        }
        else        
        {
            error( "Restore has been disabled+" );
            modes.mode = "restorewiki";
        }

    // save page
    if ( modes.mode=="save" )
    {
        if ( isset(_POST.contents) )
        {
            //contents = stripslashes( _POST.contents );
            contents = _POST.contents;

            // restricted access
            restricted=false;
            if (isLocked(title))
            {
                // check if the password is correct
                restricted=!authPassword(title, _POST.password);
                if (restricted)
                    error("Wrong password. Try again+");
            }

            // write file    
            if (!isIpBlocked() && !restricted)
                error = writeFile( title, contents );
        }
        modes.mode = "display";

        // go back if you can't write the data (avoid data loss)
        if ((restricted) || (error!=0))
            modes.mode="edit";
    }
    // cancel a page edit
    if (modes.mode=="cancel")
    {
        modes.mode = "display";
    }
    return contents;
}

// generate our html header
function htmlHeader( title, config )
{
    origTitle = title;
    if (title=="HomePage") 
      title = config.GENERAL.HOMEPAGE;  
    print("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
    print("<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"ja-JP\" lang=\"ja-JP\">\n");
    print("<head>");
    print("\n");
    print("\t<meta http-equiv=\"Content-Type\" content=\"text/html; charset="+config.GENERAL.ENCODE+"\" />\n");
    css(origTitle);
    print("\t<title>");
    if (config.GENERAL.TITLE==title)
        print(config.GENERAL.TITLE);
    else
      print(config.GENERAL.TITLE+">"+title);    
    print("</title>\n");
    print("</head>\n");
    print("<body>\n");
}
function htmlfooter2( title, config )
{
    origTitle = title;
    if (title=="HomePage") {
      title = config.GENERAL.HOMEPAGE;  
    }
    // any errors?
    for( i = 0 ; i < config.INTERNAL.ERRORS.length ; i++ ){
      err = config.INTERNAL.ERRORS[i];
      print( "<p class=\"error\">ERROR: "+err+"</p>" );
    }
    if ( getMode()=="restore" )
    {
        if (!isset(config.INTERNAL.DATA.RESTORED))
            print( "\t<form enctype=\"multipart/form-data\"  action=\""+_SERVER.SCRIPT_NAME+"?page="+title+"\" method=\"post\">\n" );
    }    
    print("\t<table width=\"100%\">\n");
    print("\t\t<tr>\n");
    print("\t\t\t<td align=\"left\"><span class=\"wiki_header\">"+title+"</span></td>\n");
    print("\t\t\t<td align=\"right\">");
    if (config.GENERAL.SHOW_CONTROLS){
        print(wikiparse( config.LOCALE.HOMEPAGE_LINK+" "+config.LOCALE.PAGELIST_LINK ) );
    }
    print( "</td>\n");
    print("\t\t</tr>\n");
    print("\t</table>\n");  

}

// generate our html footer
function htmlFooter()
{
    //global config;
    if ( config.GENERAL.DEBUG )
    {        
        list(usec, sec) = microtime().split(" ");
        end_time = (float)sec + (float)usec;
        duration = end_time - config.GENERAL.DEBUG_STARTTIME;
        uptime = shell_exec("cut -d. -f1 /proc/uptime");
        load_ar = exec("cat /proc/loadavg")).split(" ");
        load = load_ar[2];
        days = floor(uptime/60/60/24);
        hours = uptime/60/60%24;
        mins = uptime/60%60;
        secs = uptime%60;
        
        print( "<hr><b><u>DEBUG</u></b><br />" );
        print( wikiparse( "~~#FF0000:PAGE GENERATION:~~ duration secs\n" ) );    
        print( wikiparse( "~~#FF0000:SERVER UPTIME:~~ days day(s) hours hour(s) mins minute(s) and secs second(s)\n" ) );
        print( wikiparse( "~~#FF0000:SERVER LOAD:~~ load\n" ) );
    }

    print("\t</body>\n</html>\n");
}

// the start of our wiki body
function htmlStartBlock()
{
    //print("\t<hr />\n");
    print("\t<table width=\"100%\" class=\"wiki_body_container\">\n");
    print("\t\t<tr>\n");
    print("\t\t\t<td>\n");
    print("\n<!-- PAGE BODY -->\n");
}

// the end of our wiki body
function htmlEndBlock()
{
    print("<!-- END OF PAGE BODY -->\n\n");
    print("\t\t\t</td>\n");
    print("\t\t</tr>\n");
    print("\t</table>\n");
    //print("\t<hr />\n");
}

// link to another wiki page
function wikilink( title )
{
    //global config;
    if ( pageExists( title ) )
        return ("<a href=\""+_SERVER.SCRIPT_NAME+"?page="+title+"\">"+title+"</a>");
    else if ( config.SYNTAX.AUTOCREATE )
        return (title+"<a href=\""+_SERVER.SCRIPT_NAME+"?page="+title+"\">?</a>");
    else
        return (title);
}

// link to another web page
function webpagelink( text )
{
    results = text.split( "|" );
    size=results.length;
    if (size==0)
        return text;        
        
    // page link
    src=results[0];

    // link text
    desc="";
    if (size>1)
        desc = results[1];
    else
        desc = src;    
    // is our text an image?
    patterns = "/{{(.^{*)}}/";
    replacements = "\"+image( \"$1\" )+\"";    
    cmd = (" \desc = \""+desc.preg_replace( patterns, replacements )+"\";");
    eval(cmd);            

    // link target    
    window="";                
    if (size>2)
        window = results[2];
    else
    if ( config.MISC.EXTERNALLINKS_NEWWINDOW )
        window = "_blank";
    else
        window = "_self";
        
    // see whether it is a Wiki Link or not
    prefix = src.split( "/" );
    if ((prefix.length==1)) // looks like a local file, an anchor link or a wikipage
    {
        if (pageExists(src)) // is it a wiki page
        {
            src = _SERVER.SCRIPT_NAME+"?page="+src;
            window = "_self";
            resultstr = "<a href=\""+src+"\" onclick=\"target='"+window+"';\">"+desc+"</a>";
        }
        else if (src[0]=="#") // maybe its an anchor link
        {
            window = "_self";
            resultstr = "<a href=\""+src+"\" onclick=\"target='"+window+"';\">"+desc+"</a>";
        }
        else if (config.SYNTAX.AUTOCREATE) // maybe autolink
        {
            search_for_dot = src.indexOf( "." ); // don't support names with dots in - prevents creating executable scripts
            if( search_for_dot<0 )        
                resultstr = (src+"<a href=\""+_SERVER.SCRIPT_NAME+"?page="+src+"\" onclick=\"target='"+window+"';\">?</a>");
            else
                resultstr = src;
        }
        else
            resultstr = desc;
    }
    else
    {        
        resultstr = "<a href=\""+src+"\" onclick=\"target='"+window+"';\">"+desc+"</a>";            
    }
    return verbatim( resultstr );
}

// evaluate a chunk of text
function wikiEval( str )
{
    result = "";
    cmd = "result = \""+str+"\";";
    eval(cmd);
    return result;
}

// colour some text
function colouredtext( text )
{
    results = text.split( ":" );
    size=results.length;
    if (size<2)
        return text;        
    colour=results[0];
    contents = wikiEval(implode(":", array_slice( results, 1)));
    resultstr = "<span style=\"color: #"+colour+";\">"+contents+"</span>";
    return verbatim( resultstr );
}

// place an image
function image( text )
{    
  results = text.split( "|" );
  size=results.length;
  src="";
  desc="";
  width='';
  height='';
  align="";
  valign="";
  if (size>=1)
     src = " src=\""+results[0]+"\"";
  if (size>=2)
     desc = " alt=\""+results[1]+"\"";
  else
     desc = " alt=\"[img]\"";
  if (size>=3)
     width += " width: "+results[2]+"px;";
  if (size>=4)
     height += " height: "+results[3]+"px;";
  if (size>=5)
     align = " float: "+results[4]+";";
  if (size>=6)
     valign=" vertical-align: "+results[5]+";";
  resultstr="";
  if (size>0)
     resultstr = "<img" + src + " style=\"border:0pt none;" + width + height + align + valign + "\"" + desc + " />";
  return verbatim( resultstr );
}

// get some verbatim text
function getVerbatim( index )
{
    //global config;
    verbat = config.INTERNAL.VERBATIM;
    return verbat[index];
}

// store some verbatim text
function verbatim( contents )
{
    //global config;
    verbat = config.INTERNAL.VERBATIM;
    index = verbat.length;
    verbat[index] = contents;
    return "\"+getVerbatim("+index+")+\"";
}

// replace special chars with the appropriate html
function htmltag( contents )
{
    // ' must be used for fields
    result = contents;
    result.replace ("&lt;", "<");
    result.replace ("&gt;", ">");
    result.replace ("&quot;", "\\\"");
    return result;
}

// parse wiki code & replace with html
function wikiparse( contents )
{
    //global config;
    //patterns = [];
    //replacements = [];
    patterns = [];
    replacements = [];
    //contents = htmlspecialchars(contents, ENT_COMPAT, "UTF-8");
    contents = htmlspecialchars(contents.addSlashes());
    // webpage links
    patterns[0] = "/\\[\\[([^\\[]*)\\]\\]/";
    replacements[0] = "\"+webpagelink( \"$1\" )+\"";        

    // images
    patterns[1] = "/{{([^{]*)}}/";
    replacements[1] = "\"+image( \"$1\" )+\"";    

    // coloured text
    patterns[2] = "/~~#([^~]*)~~/";
    replacements[2] = "\"+colouredtext( \"$1\" )+\"";    
    
    patterns[3] = '/\\$/';
    replacements[3] = "&DOLLAR;";
    
    // verbatim text
    patterns[4] = "/~~~(.*)~~~/";
    replacements[4] = "\"+verbatim( \"$1\" )+\"";

    if ( config.SYNTAX.HTMLCODE )
    {
        patterns[5] = "/%%(.*)%%/";
        replacements[5] = "\"+htmltag( \"$1\" )+\"";        
    }

    // substitute complex expressions
    contents = wikiEval( contents.preg_replace( patterns, replacements ) );
    //contents = contents.preg_replace( patterns, replacements );
    //contents = wikiEval( contents );
    patterns = [];//[];
    replacements = [];//array();

    // h1
    patterns[0] = "/==([^=]*[^=]*)==/";
    replacements[0] = "<h1>$1</h1>";

    // italic
    patterns[1] = "/''([^']*[^']*)''/";
    replacements[1] = "<i>$1</i>";

    // bold
    patterns[2] = "/\\*\\*([^\\*]*[^\\*]*)\\*\\*/";
    replacements[2] = "<b>$1</b>";

    // underline
    patterns[3] = "/__([^_]*[^_]*)__/";
    replacements[3] = "<span style=\\\"text-decoration: underline;\\\">$1</span>";    

    // html shortcuts
    patterns[4] = "/@@([^@]*)@@/";
    replacements[4] = "<a name=\\\"$1\\\"></a>";
    
    // wiki words    
    if ( config.SYNTAX.WIKIWORDS )
    {
        patterns[5] = "/([A-Z][a-z0-9]+[A-Z][A-Za-z0-9]+)/";
        replacements[5] = "\"+wikilink( \"$1\" )+\"";    
    }

    // substitute simple expressions & final expansion
    contents = wikiEval( contents.preg_replace( patterns, replacements ) );
    //contents = contents.preg_replace( patterns, replacements );
    patterns = [];//array();
    replacements = [];//array();

    // replace some whitespace bits & bobs  
    patterns[0] = "/\t/";
    replacements[0] = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
    patterns[1] = "/  /";
    replacements[1] = "&nbsp;&nbsp;";
    patterns[2] = "/&DOLLAR;/";
    replacements[2] = "$";
    patterns[3] = "/\n/";
    replacements[3] = "<br>\n";
    contents = contents.preg_replace( patterns, replacements );
    return contents;
}

// returns the directory where the wiki pages are stored
function pageDir()
{
    //global config;
    return (config.GENERAL.PAGES_DIRECTORY);
}

// returns the directory where the temporary backups are stored
function tempDir()
{
    //global config;
    return (config.GENERAL.TEMP_DIRECTORY);
}

// returns the full path to a page
function pagePath( title )
{
    return (pageDir()+title.nkfconv("Ws"));
}

// clean up the temp directory
function cleanupTempFiles()
{
    files = wikiReadDir(tempDir());
    for( i=0 ; i < files.length ;i++){
        mtime = FILE.filedate( file[i] );
        //now = date("U");
        //if ( now-mtime>300 ) // delete any files that are older than 5 minutes
        //    unlink( file ); 
    }
}

// is this page 'special'?
function isSpecial( title )
{
    //global config;
    return ( isset( config.SPECIAL[title] ) );
}

// is this page 'locked'?
function isLocked( title )
{
    //global config;
    return ( isset( config.RESTRICTED[title] ) );
}



// add an error to our error buffer
function error( str )
{
    //global config;
    config.INTERNAL.ERRORS[] = str;
}

// are there any errors so far?
function anyErrors()
{
    //global config;
    if (config.INTERNAL.ERRORS.length==0)
        return false;
    else
        return true;
}

// is this ip address blocked?
function isIpBlocked( )
{
    //global config;
    result = false;
    ipaddress = _SERVER.REMOTE_ADDR;  
    //foreach (config.BLOCKED_IPS as ip)
    //{
    //    if (preg_match( "/"+ip+"/", ipaddress ))
    //    {
    //        error( "Your ip address has been blocked from making changes!" );
    //        result = true;
    //        break;
    //    }    
    //}
    return result;
}

// does a given page exist yet?
function pageExists( title )
{
    if (FILE.file_exists( pagePath( title ) ) || isSpecial( title ) ){
        return true;
    }else{
        return false;
    }
}

// returns a list of pages
function pageList()
{
    //global config;
    contents = "";
    files = wikiReadDir(pageDir());
    details = [];
    for( i = 0 ; i < files.length ; i++ ){
        file = files[i];
        details[file] = FILE.filedate( file );
    }
    //arsort(details);
    //reset(details);
    s = Object.keys(details);  
    for( i=0 ; i<s.length ; i++ ){
         dd = details[s[i]];
         ff = basename(s[i]).nkfconv("Sw");
         contents += "[["+basename(s[i]).nkfconv("Sw")+"]]\t"+details[s[i]].toDateString(config.GENERAL.MODTIME_FORMAT)+"\n";
    }
    return contents;
}

// returns the pageList in RSS2.0 format
function rssFeed()
{
    print( "<?xml version=\"1.0\"?>\n" );
    print( "<rss version=\"2.0\">\n" );
    print( "\t<channel>\n" );
    title = config.GENERAL.TITLE;
    print( "\t\t<title>title</title>\n" );
    url = "http://"+_SERVER.SERVER_NAME._SERVER.SCRIPT_NAME;
    print( "\t\t<link>"+url+"</link>\n" );
    print( "\t\t<description>Recently changed pages on the title wiki.</description>\n" );
    print( "\t\t<generator>Wiki v"+config.PAWFALIKI_VERSION+"</generator>\n" );    
    files = wikiReadDir(pageDir());
    details = [];
    for( i = 0 ; i < files.length ; i++ ){
        file = files[i];
        details[file] = FILE.filedate( file );
    }
    //arsort(details);
    //reset(details);
    item = 0;
    numItems = config.RSS.ITEMS;
    while ( list(key, val) = each(details) )      
    {
        title = basename(key);
        modtime = date( config.RSS.MODTIME_FORMAT, val );
        description = title+" "+modtime;
        print( "\t\t<item>\n" );
        if (config.RSS.TITLE_MODTIME)             
            print( "\t\t\t<title>description</title>\n" );
        else
            print( "\t\t\t<title>title</title>\n" );
        print( "\t\t\t<link>"+url+"?page=title</link>\n" );    
        print( "\t\t\t<description>"+description+"</description>\n" );
        print( "\t\t</item>\n" );    
        item++;
        if (numItems!=-1&&item>=numItems)
            break;
    }
    print( "\t</channel>\n" );
    print( "</rss>\n" );
}

// backup all the wiki pages to a file
function backupPages( filename )
{    
    files = wikiReadDir(pageDir());
    details = [];
    for( i = 0 ; i < files.length ; i++ ){
        file = files[i];
        details[file] = FILE.filedate( file );
    }
    //arsort(details);
    //reset(details);    
    pages = [];
    pos = 0;
    while ( list(key, val) = each(details) )      
    {
        pages[pos] = [];
        pages[pos].title = basename(key);
        pages[pos].datestring = date("U", val );
        pos = pos+1;
    }    
    numpages = pages.length;
    if (numpages==0) // must have at least 1 page for a backup
    {
        error("No pages to backup yet!");
        return;
    }
    if ( extension_loaded('zlib')&&config.BACKUP.USE_ZLIB ) // write a gzipped backup file
    {
        filename = filename+"+gz";
        zp = gzopen(filename, "w9");
        gzwrite(zp, numpages+"\n");
        for( i = 0 ; i < pages.length ; i++ ){
            page = pages[i];
            contents = page.title+"\n"+page.datestring+"\n";
            lines = rtrim( wikiReadFile( pagePath( page.title ) ) );
            numlines = lines.split("\n").length;
            if (numlines==0) // no lines?! weird - we must have at least 1 line for restore
            {
                numlines=1;
                lines.="\n";
            }
            contents += numlines+"\n"+lines+"\n";
            gzwrite( zp, contents );
        }
        gzclose(zp);    
    }
    else // otherwise normal binary file
    {
        fd = fopen( filename, "wb" );
        fwrite( fd, numpages+"\n" );
        for( i = 0 ; i < pages.length ; i++ ){
            page = pages[i];
            contents = page.title+"\n"+page.datestring+"\n";
            lines = rtrim( wikiReadFile( pagePath( page.title ) ) );
            numlines = lines.split("\n").length;
            if (numlines==0) // no lines?! weird - we must have at least 1 line for restore
            {
                numlines=1;
                lines.="\n";
            }
            contents += numlines+"\n"+lines+"\n";
            fwrite( fd, contents );
        }
        fclose( fd );    
    }
    return 0;
}

// restore all the wiki pages from a file
function restorePages()
{
    //global config, _FILES;
    unset(config.INTERNAL.DATA.RESTORED);
    if (!authPassword("RestoreWiki", _POST.password))
    {
        error("Wrong password. Try again+");
        return;
    }
    
    filename = _FILES.userfile.tmp_name;
    if (filename=="none"||_FILES.userfile.size==0||!is_uploaded_file(filename))
    {
        error( "No file was uploaded!<BR>Maybe the filesize exceeded the maximum upload size of "+config.BACKUP.MAX_SIZE+"bytes+" );
        return;
    }
    
    // if we can use zlib functions - they can read uncompressed files as well
    zlib = false;
    if ( extension_loaded('zlib')&&config.BACKUP.USE_ZLIB ) zlib = true;

    // sanity check on file
    if (zlib)
        fd = gzopen(filename, "rb9");
    else
        fd = fopen(filename, "rb");
    if (!fd)
    {
        error("Could not read temporary upload file: filename!");
        return;
    }                
    fileerror = "NO ERROR";
    if (zlib)
        numPages = trim(gzgets(fd));
    else
        numPages = trim(fgets(fd));
        
    if (numPages>0) // must be at least 1 page
    {
        for (i=0; i<numPages; i++)
        {
            if (zlib)
            {
                @gzgets(fd); if (gzeof(fd)) {fileerror="GZ: Invalid title on page i!";} // read title
                @gzgets(fd); if (gzeof(fd)) {fileerror="GZ: Invalid mod time on page i!";} // mod time
                numLines = trim(gzgets(fd)); // num lines
            }
            else
            {
                @fgets(fd); if (feof(fd)) {fileerror="Invalid title on page i!";} // read title
                @fgets(fd); if (feof(fd)) {fileerror="Invalid mod time on page i!";} // mod time
                numLines = trim(fgets(fd)); // num lines
            }        
                            
            if (numLines>0) // must have at least 1 line
            {
                for (j=0; j<numLines; j++)
                {
                    if (zlib)
                    {
                        @gzgets(fd); if (gzeof(fd)&&i!=numPages-1) {fileerror="GZ: Invalid line read on page i!";} // page content
                    }
                    else
                    {
                        @fgets(fd); if (feof(fd)&&i!=numPages-1) {fileerror="Invalid line read on page i!";} // page content
                    }
                }
            }
            else
            {
                fileerror = "Invalid number of page lines on page i!";
            }
        }
    }
    else
    {
        fileerror = "Invalid number of backup pages!";
    }
    if (zlib)
        gzclose(fd);
    else
        fclose(fd);        
    if (fileerror!="NO ERROR")
    {
        str = "This does not appear to be a valid backup file!";
        if(!zlib)
            str += "<br />NOTE: Zlib is not enabled so restoring a compressed file will not work+";
        error(str);
        return;
    }        
    
    // if we got here the file is OK - restore the pages!!
    restored = config.INTERNAL.DATA.RESTORED;
    restored = [];        
    if (zlib)
        fd = gzopen(filename, "rb9");
    else
        fd = fopen(filename, "rb");
    if (zlib)
        numPages = trim(gzgets(fd));
    else
        numPages = trim(fgets(fd));
    for (i=0; i<numPages; i++)
    {
        if (zlib)
        {
            title = trim(gzgets(fd));
            modtime = trim(gzgets(fd));
            numLines = trim(gzgets(fd));
            contents = "";
            for (j=0; j<numLines; j++)
                contents += gzgets(fd);        
        }
        else
        {
            title = trim(fgets(fd));
            modtime = trim(fgets(fd));
            numLines = trim(fgets(fd));
            contents = "";
            for (j=0; j<numLines; j++)
                contents += fgets(fd);
        }
        if (!writeFile(title, contents))
        {
            if (@touch(pagePath( title ), modtime, modtime)==false)
            {
                error("Could not modify filetimes for title - ensure php owns the file!");
            }
            restored[] = title;
        }
    }
    if (zlib)
        gzclose(fd);
    else
        fclose(fd);    
}

// print a little wiki syntax box
function printWikiSyntax()
{
    print("\t<div class=\"wikisyntax\">\n");
    print("\t<table>\n");
    print("\t\t<tr>\n");
    print("\t\t\t<td colspan=3>");
    print(wikiparse("**__Syntax__** ")+"<span class=\"optionalvalue\">(optional values)</span><br />");
    print("\t\t\t</td>\n");
    print("\t\t</tr>\n");
    print("\t\t<tr>\n");
    print("\t\t\t<td align=\"right\">");
    print( "bold text: <br />" );
    print( "italic text: <br />" );
    print( "underlined text: <br />" );
    print( "verbatim(無効) text: <br />" );
    print( "link: <br />" );
    if ( config.SYNTAX.WIKIWORDS )
        print( "wiki link: <br />" );
    print( "image: <br />" );
    print( "hex-coloured text: <br />" );
    if ( config.SYNTAX.HTMLCODE )
        print( "html code: <br />" );
    print( "anchor link: <br />" );
    print("\t\t\t</td>\n");
    print("\t\t\t<td>");
    print( "**abc**<br />" );
    print( "''abc''<br />" );
    print( "__abc__<br />" );
    print( "~~~abc~~~<br />" );
    print( "[[url|<span class=\"optionalvalue\">description</span>|<span class=\"optionalvalue\">target</span>]]<br />" );
    if ( config.SYNTAX.WIKIWORDS )
        print( "WikiWord<br />" );
    print( "{{url|<span class=\"optionalvalue\">alt</span>|<span class=\"optionalvalue\">width</span>|<span class=\"optionalvalue\">height</span>|<span class=\"optionalvalue\">align</span>|<span class=\"optionalvalue\">vertical-align</span>}}<br />" );
    print( "~~#AAAAAA:grey~~<br />" );
    if ( config.SYNTAX.HTMLCODE )
        print( "%%html code%%<br />" );
    print( "@@name@@<br />" );
    print("\t\t\t</td>\n");
    print("\t\t</tr>\n");
    print("\t</table>\n");
    print("\t</div>\n");
}

// display a wiki page
//function displayPage( title, mode, contents="" )
function displayPage( title, mode, contents )
{     
    //global config;

    // handle special pages 
    if( title == "PageList" ) {
            contents = pageList();
    }else if ( title == "RestoreWiki" ){        
            if ( !isset(config.INTERNAL.DATA.RESTORED) )
            {            
                contents += "<b>WARNING: Restoring wiki pages will overwrite any existing pages with the same name!</b><br /><br />" ;
                contents += "Backup File: ";    
                contents += "<input type=\"hidden\" name=\"MAX_FILE_SIZE\" value=\""+config.BACKUP.MAX_SIZE+"\" /><br />";
                contents += "<input name=\"userfile\" type=\"file\" class=\"fileupload\" size=\"32\" /><br /><br />";
                contents += "Enter the password below and click <b>restore</b>+";
            }
            else
            {
                contents = wikiparse("Restored **"+config.INTERNAL.DATA.RESTORED.length+"** wiki pages:\n");
                for( i = 0 ; config.INTERNAL.DATA.RESTORED.length ; i++ ){
                    page = config.INTERNAL.DATA.RESTORED[i];
                    contents += wikiparse("-> [["+page+"]]\n");
                }
            }
    }else if ( title == "BackupWiki" ){
            if (!anyErrors())
            {
                wikiname = config.GENERAL.TITLE;
                wikiname.replace( " ", "_" );
                files = wikiReadDir(pageDir());
                backups = wikiReadDir(tempDir());
                contents = "Backed up "+files.length+" pages+\n\nRight-click on the link below and \"Save Link to Disk...\".\n";
            }
    }else{
            if ( pageExists( title ) )
            {
                if (!( (mode=="edit") && (contents!="") )){
                    contents = wikiReadFile( pagePath( title ) );
                }
            }
            else
            {
                contents = "This is the page for "+title+"!";
                mode = "editnew";
            }
    }
    
    if       ( mode == "display" ){
        print("<span class=\"wiki_body\">\n");
        print( wikiparse( contents ) );
        print("</span>\n");
    }else if( mode == "backupwiki" ){
        print("<span class=\"wiki_body\">\n");
        print( wikiparse( contents ) );
        print("</span>\n");
    }else if( mode ==  "restorewiki" ){ 
        print("<span class=\"wiki_body\">\n");
        print( contents );
        print("</span>\n");
    }else if( mode == "edit" || mode == "editnew" ){
        print( "<form action=\""+_SERVER.SCRIPT_NAME+"?page="+title+"\" method=\"post\">\n" );
        print( "<textarea name=\"contents\" cols=\"80\" rows=\"48\">"+contents+"</textarea>\n" );    
    }
    return mode;
}

// display the wiki controls
function displayControls( title, mode )
{
    //global config;
    print("\t<table width=\"100%\">\n");
    print("\t\t<tr>\n");
    print("\t\t\t<td align=\"left\" valign=\"top\">\n"); 
    if (config.GENERAL.SHOW_CONTROLS)
    {
        if( mode == "display" ){
                if (!(isSpecial(title)))
                {
                    print( "\t\t\t\t<form action=\""+_SERVER.SCRIPT_NAME+"?page="+title+"\" method=\"post\">\n" );
                    print( "\t\t\t\t\t<p>\n" );
    
                    if( config.MISC.REQ_PASSWORD_TEXT_IN_EDIT_BTN )
                    {
                        print( "<input name=\"mode\" value=\"edit\" type=\"hidden\" />");
                        print( "\t\t\t\t\t\t<input value=\"edit " );
                        if (isLocked(title))
                            print(config.LOCALE.REQ_PASSWORD);
                        print( "\" type=\"submit\" />");
                    }
                    else
                    {
                        print( "\t\t\t\t\t\t<input name=\"mode\" value=\"edit\" type=\"submit\" />" );
                        if (isLocked(title))
                            print( wikiparse(config.LOCALE.REQ_PASSWORD));        
                    } 
            
                    print( "\n\t\t\t\t\t</p>\n" );
                    print( "\t\t\t\t</form>\n" );
                }
                if (title=="PageList"&&config.BACKUP.ENABLE)
                {                
                    print( "\t\t\t\t<form action=\""+_SERVER.SCRIPT_NAME+"?page="+title+"\" method=\"post\">\n" );
                    print( "\t\t\t\t\t<p>\n" );
                    print( "\t\t\t\t\t\t<input name=\"mode\" value=\"backup\" type=\"submit\" />" );
                    print( "\t\t\t\t\t\t<input name=\"mode\" value=\"restore\" type=\"submit\" />" );
                    print( "\n\t\t\t\t\t</p>\n" );
                    print( "\t\t\t\t</form>\n" );
                }
        }else if( mode == "backupwiki" ){
                if (!anyErrors())
                {
                    wikiname = config.GENERAL.TITLE;
                    wikiname.replace( " ", "_" );
                    files = wikiReadDir(pageDir());
                    backups = wikiReadDir(tempDir());
                    details = [];
                    for( i = 0 ; i < backups.length ; i++ ){
                        backup = backups[i];
                        details[backup] = FILE.filedate( backup );
                    }
                    //arsort(details);
                    //reset(details);    
                    while ( list(key, val) = each(details) )      
                    {
                        size = filesize(key);
                        print( wikiparse("[[key|"+basename(key)+"]] (size bytes)\n"));
                    }
                }
        }else if( mode == "restorewiki" ){
                if ( !isset(config.INTERNAL.DATA.RESTORED) )
                {
                    print( "\t\t\t\t\t<p>\n" );
                    print(wikiparse(" "+config.LOCALE.PASSWORD_TEXT)); 
                    print("<input name=\"password\" type=\"password\" class=\"pass\" size=\"17\" />");
                    print( "\t\t\t\t\t<input name=\"mode\" value=\"restorewiki\" type=\"hidden\" />\n" );    
                    print( "\t\t\t\t\t<input value=\"restore\" type=\"submit\" />\n" );        
                    print( "\t\t\t\t\t</p>\n" );
                }
        }else if( mode == "edit" ){
                print( "\t\t\t\t\t<p>\n" );
                if (isLocked(title))
                {
                    print(wikiparse(config.LOCALE.PASSWORD_TEXT)); 
                    print("<input name=\"password\" type=\"password\" class=\"pass\" size=\"17\" />");
                }
                print( "\t\t\t\t\t<input name=\"mode\" value=\"save\" TYPE=\"submit\" />\n" );
                print( "\t\t\t\t\t<input name=\"mode\" value=\"cancel\" TYPE=\"submit\" />\n" );
                print( "\t\t\t\t\t</p>\n" );
                print( "\t\t\t\t</form>\n" );
        }else if( mode == "editnew" ){
                print( "\t\t\t\t\t<p>\n" );
                if (isLocked(title))
                {
                    print(wikiparse(config.LOCALE.PASSWORD_TEXT)); 
                    print("<input name=\"password\" type=\"password\" class=\"pass\" size=\"17\" />");
                }
                print( "\t\t\t\t\t\t<input name=\"mode\" value=\"save\" type=\"submit\" />" );
                print( "\t\t\t\t\t</p>\n" );
                print( "\t\t\t\t</form>\n" );
        }
    }
    print("\t\t\t</td>\n");
    print("\t\t\t<td align=\"right\" valign=\"top\">\n");
    print("\t\t\t\t<p>\n");
    print("\t\t\t\t</p>\n");
    print("\t\t\t</td>\n");
    print("\t\t</tr>\n");
    print("\t</table>\n");
    if ( getMode()=="restore" )
        print( "\t</form>\n" );
    if ( (mode=="edit"||mode=="editnew") && config.SYNTAX.SHOW_BOX && title!="RestoreWiki" )
        printWikiSyntax();
}
// パッチ
function mybasename(str){
    pos = str.indexOf(str,"/");
    if( pos < 0 ){
        return str;
    }else{
        return substr(str,pos+1,strlen(str)-pos-1);
    }
}
//==============
//==============
// MAIN BLOCK!
//==============
//==============

// by defining PAWFALIKI_FUNCTIONS_ONLY and including this file we can use all
// the wiki functions without actually displaying a wiki.
if (!isset(PAWFALIKI_FUNCTIONS_ONLY))
{
    if (config.GENERAL.DEBUG)
    {
        list(usec, sec) = microtime().split(" ");
        config.GENERAL.DEBUG_STARTTIME = (float)sec + (float)usec;
    }

    // stop the page from being cached
    header("Cache-Control: no-store, no-cache, must-revalidate");

    // find out what wiki 'mode' we're in
    mode = getMode();
    format = _GET.format;
    if ( format=="rss" && config.RSS.ENABLE )
    {
        rssFeed();
        exit();
    }
    
    // get the page title
    title = getTitle();

    if ( mode=="backup" )
        title = "BackupWiki";
    if ( mode=="restore" )
        title = "RestoreWiki";

    // get the page contents
    modes={};
    modes.mode = mode;
    contents = updateWiki( modes, title, config );
    mode = modes.mode;

    // page header
    if (mode=="edit") 
        htmlHeader(wikiparse(config.LOCALE.EDIT_TITLE)+title, config); 
    else
        htmlHeader(title, config); 

    // page contents
    htmlStartBlock();
    mode = displayPage(title, mode, contents);
    htmlEndBlock();

    // page controls
    displayControls(title, mode);
    if (mode=="edit") {
        htmlfooter2(wikiparse(config.LOCALE.EDIT_TITLE)+title, config); 
    }else{
        htmlfooter2(title, config);
    }

    // page footer
    htmlFooter();
}
?>
