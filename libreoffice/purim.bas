REM  *****  BASIC  *****

Sub Main
 Call EndDocLoad_event_handler
End Sub

REM  *****  BASIC  *****

Public dispathcer As Object
Public thisFrame As Object

'-----------------------------------------------------------------
Sub Main_Comm_Loop
Dim sNumber As Integer, cNumber As Integer, req As Integer
Dim sLine As String
Dim SrvFile, ClientFile As String, destFile As String, templateFile As String
Dim sMsg As String, giver As String, families As String
Dim finish As Boolean
Dim lastRef As Integer, newRef As Integer

    SrvFile = "/home/mwiener/purimsrv.txt"
    ClientFile = "/home/mwiener/purimclient.txt"
    finish = False
    lastRef = -1
    
    'Create the server file
    sNumber = Freefile
    Open SrvFile For Append As #sNumber
    Close #sNumber
    
    Do
	    'Read - waiting command
	    While FileExists(ClientFile) = False
	    	Wait(1000)
	    Wend
	    While FileLen(ClientFile) < 2
	    	Wait(1000)
	    Wend
	    cNumber = Freefile
	    Open ClientFile For Input As #cNumber
	    Line Input #cNumber, sLine
	    Close #cNumber   
	    'Empty server file
	    sNumber = Freefile
	    Open SrvFile For Output As #sNumber
	    Close #sNumber
	    req = Val(mid(sLine, 1, 3))
	    newRef = Val(mid(sLine, 5, 4))
	    if newRef <> lastRef Then
		    lastRef = newRef
		    Select Case req
		    	case 1	'set template file name
		    		templateFile = mid(sLine, 10, Len(sLine)-9)
		    		sMsg = mid(sLine, 1, 4) & Format(mid(sLine, 5, 4),"0000") & ":ACK"  & chr(10)
		    	case 2	'set notes file name
		    		destFile = mid(sLine, 10, Len(sLine)-9)
		    		Call empty_dest_file( destFile )
		    		sMsg = mid(sLine, 1, 4) & Format(mid(sLine, 5, 4),"0000") & ":ACK"  & chr(10)
		    	case 3 'giver record (giver name,receiver #1 name, receiver #2 name ...
		    		Call parseNamesList( Mid(sLine, 10, Len(sLine)-9), giver, families )
		    		Call Add_Note(DestFile, templateFile, giver, families )
		    		'test_make_notes
		    		sMsg = mid(sLine, 1, 4) & Format(mid(sLine, 5, 4),"0000") & ":ACK" & chr(10)
		    	case 4 'finish
		    		sMsg = "goodby" & chr(10)
		    		finish = True
		    	case Else
		    End Select    
		    'Write - response
		    sNumber = Freefile
		    Open SrvFile For Output As #sNumber
		    Print #sNumber, sMsg
		    Close #sNumber
	    End If
	    Wait(1000)
	Loop Until finish = True

    Exit Sub
    
End Sub

'-----------------------------------------------------------------
Sub set_current_doc( ByVal FileName As String )
	dim document   as object ' this document
	Dim dispatcher As Object
	Dim sUrl As String 
	dim args(1) as new com.sun.star.beans.PropertyValue
	
	sUrl = convertToUrl( FileName )
	document = stardesktop.LoadComponentFromURL(sUrl, "_blank", 0, Array())
	thisFrame = document.CurrentController.Frame
	dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
	
	args(0).Name = "Count"
	args(0).Value = 1
	args(1).Name = "Select"
	args(1).Value = false
	dispatcher.executeDispatch(thisFrame, ".uno:GoDown", "", 0, args())
	dispatcher.executeDispatch(thisFrame, ".uno:JumpToHeader", "", 0, Array())
	dispatcher.executeDispatch(thisFrame, ".uno:Escape", "", 0, Array())
	dispatcher.executeDispatch(thisFrame, ".uno:JumpToFooter", "", 0, Array())
	dispatcher.executeDispatch(thisFrame, ".uno:GoToStartOfDoc", "", 0, Array())
End Sub

REM -------------------------------------------------
sub compare_files

dim sUrl as string
dim oDoc as object, oDocFrame as object, dispatcher as object
dim PropVal(0) as new com.sun.star.beans.PropertyValue
dim args(0) as new com.sun.star.beans.PropertyValue

sUrl = convertToUrl("/home/mwiener/Documents/source.odt")

oDoc = stardesktop.LoadComponentFromURL(sUrl, "_blank", 0, Array())

oDocFrame = oDoc.CurrentController.Frame
dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")

PropVal(0).Name = "URL"
PropVal(0).Value = convertToUrl("/home/mwiener/Documents/dest.odt")

dispatcher.executeDispatch(oDocFrame, ".uno:CompareDocuments", "", 0, PropVal())

args(0).Name = "ShowTrackedChanges"
args(0).Value = true
dispatcher.executeDispatch(oDocFrame, ".uno:ShowTrackedChanges", "", 0, args())

'dispatcher.executeDispatch(oDocFrame, ".uno:AcceptChanges", "", 0, array())'

end sub

'----------------------------------------------------------------------
sub copy_current_file_into_another( ByVal DestFileName As String )
dim document   as object ' this document
dim ourFrame as Object
dim dispatcher2 as object
dim ourUrl As String, otherUrl as string
dim oDoc as object, oDocFrame as object
dim oCursor as object
dim PropVal(0) as new com.sun.star.beans.PropertyValue
dim args(1) as new com.sun.star.beans.PropertyValue

ourUrl = thisComponent.GetUrl()
document = stardesktop.LoadComponentFromURL(ourUrl, "_blank", 0, Array())
thisFrame = document.CurrentController.Frame
dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")

args(0).Name = "Count"
args(0).Value = 1
args(1).Name = "Select"
args(1).Value = false
dispatcher.executeDispatch(thisFrame, ".uno:GoDown", "", 0, args())
dispatcher.executeDispatch(thisFrame, ".uno:JumpToHeader", "", 0, Array())
dispatcher.executeDispatch(thisFrame, ".uno:SelectAll", "", 0, Array())
dispatcher.executeDispatch(thisFrame, ".uno:Copy", "", 0, Array())

if FileExists( DestFileName ) = False Then
	Call create_empty_file( DestFileName )
End If

otherUrl = convertToUrl( DestFileName )
oDoc = stardesktop.LoadComponentFromURL(otherUrl, "_blank", 0, Array())
oDocFrame = oDoc.CurrentController.Frame

dispatcher2 = createUnoService("com.sun.star.frame.DispatchHelper")
REM bring cursor to tail of destination document
dispatcher2.executeDispatch(oDocFrame, ".uno:JumpToFooter", "", 0, Array())
dispatcher2.executeDispatch(oDocFrame, ".uno:Paste", "", 0, Array())
REM add a page break so the next appending starts on a new page
dispatcher2.executeDispatch(oDocFrame, ".uno:InsertPagebreak", "", 0, Array())
dispatcher2.executeDispatch(oDocFrame, ".uno:Save", "", 0, Array())

'Close other document
'oDocFrame.Close(False)

REM bring focus back to our document
dispatcher.executeDispatch(ourFrame, ".uno:Escape", "", 0, Array())
dispatcher.executeDispatch(ourFrame, ".uno:JumpToFooter", "", 0, Array())
dispatcher.executeDispatch(ourFrame, ".uno:GoToStartOfDoc", "", 0, Array())

'ourFrame.Close(False)
end sub


sub goDown
rem ----------------------------------------------------------------------
rem define variables
dim document   as object
dim dispatcher as object
rem ----------------------------------------------------------------------
rem get access to the document
document   = ThisComponent.CurrentController.Frame
dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:GoToNextPara", "", 0, Array())

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:JumpToFooter", "", 0, Array())

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:GoToNextPara", "", 0, Array())

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:JumpToFooter", "", 0, Array())


end sub


sub page_break
rem ----------------------------------------------------------------------
rem define variables
dim document   as object
dim dispatcher as object
rem ----------------------------------------------------------------------
rem get access to the document
document   = ThisComponent.CurrentController.Frame
dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:InsertPagebreak", "", 0, Array())


end sub


sub Esc
rem ----------------------------------------------------------------------
rem define variables
dim document   as object
dim dispatcher as object
rem ----------------------------------------------------------------------
rem get access to the document
document   = ThisComponent.CurrentController.Frame
dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:Escape", "", 0, Array())


end sub

'------------------------------------------------------------------------
Sub create_empty_file( ByVal fileName as String )
	Dim oSFA As Object, oOutStream as Object, oOutText as Object

	oSFA = createUNOService ("com.sun.star.ucb.SimpleFileAccess")
    If oSFA.exists(fileName) Then
      oSFA.kill(fileName) 'if file exists then delete it
    End If
   
   oOutStream = oSFA.openFileWrite(fileName)
   oOutText = createUNOService ("com.sun.star.io.TextOutputStream")
   oOutText.setOutputStream(oOutStream)

   oOutText.WriteString("")
   oOutText.closeOutput()
End Sub

'------------------------------------------------------------------
Sub create_new_doc( ByVal fileName As String )
	Dim dispatcher As Object, document As Object	
	dim args(0) as new com.sun.star.beans.PropertyValue
	
	dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
	document   = ThisComponent.CurrentController.Frame
	
	dispatcher.executeDispatch(document, ".uno:NewDoc", "", 0, Array())
End Sub

rem ----------------------------------------------------------------------
sub middle_align_current_file
	Dim dispatcher As Object, document As Object
	
	dim args2(0) as new com.sun.star.beans.PropertyValue
	dim args3(0) as new com.sun.star.beans.PropertyValue
	
	Call Initialize
	dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
	document   = ThisComponent.CurrentController.Frame
	dispatcher.executeDispatch(document, ".uno:SelectAll", "", 0, Array())
	
	args2(0).Name = "CenterPara"
	args2(0).Value = true
	dispatcher.executeDispatch(document, ".uno:CenterPara", "", 0, args2())
	
	args3(0).Name = "ParaRightToLeft"
	args3(0).Value = true
	dispatcher.executeDispatch(document, ".uno:ParaRightToLeft", "", 0, args3())
end sub

'----------------------------------------------------------------------
Sub LoadNewCalcWorkbook
   Dim Doc As Object
   Dim NewWorkbook As Object
   Dim oDispatch As Object
   Dim Url As String
   Dim Args(1) As new com.sun.star.beans.PropertyValue
   Dim s As String
   
   Doc = ThisComponent
   
   Url = "file:///home/mwiener/cfiles/ipc/newWorkbook.ods"
       
   Args(0).Name = "Hidden"
   Args(0).Value = False
   Args(1).Name = "MacroExecutionMode"
   Args(1).Value = 4
       
   NewWorkbook = StarDesktop.loadComponentFromURL("private:factory/scalc", "_Blank", 0, Args)
   
   Args(0).Name = "FilterName"
   Args(0).Value = "calc8"
   Args(1).Name = "Overwrite"
   Args(1).value = True
   
   NewWorkbook.storeAsURL(Url,Args)            
End Sub

'----------------------------------------------------------------------
Sub LoadNewWriterDoc( ByVal filename As String )
   Dim Doc As Object
   Dim newWriterDoc As Object
   Dim Url As String
   Dim Args(1) As new com.sun.star.beans.PropertyValue
   Dim s As String
   
   Doc = ThisComponent   
   Url = convertToUrl( filename )
       
   Args(0).Name = "Hidden"
   Args(0).Value = False
   Args(1).Name = "MacroExecutionMode"
   Args(1).Value = 4
       
   newWriterDoc = StarDesktop.loadComponentFromURL("private:factory/swriter", "_Blank", 0, Args)
   
   Args(0).Name = "FilterName"
   Args(0).Value = "writer8"
   Args(1).Name = "Overwrite"
   Args(1).value = True
   
   newWriterDoc.storeAsURL(Url,Args)  
   newWriterDoc.Close(False)          
End Sub


'--------------------------------------------------------------------
Sub Initialize
	dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
End Sub

sub Hello
rem ------------------------------------------
'Write "Hello world" and a NewLine char in this current document at the
'position of the cursor
	dim document   as object
	dim dispatcher as object
	
	rem get access to the document
	document   = ThisComponent.CurrentController.Frame
	dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
	
	rem ----------------------------------------------------------------------
	dim args1(0) as new com.sun.star.beans.PropertyValue
	args1(0).Name = "Text"
	args1(0).Value = "Hello world"
	
	dispatcher.executeDispatch(document, ".uno:InsertText", "", 0, args1())
	'put NewLine
	dispatcher.executeDispatch(document, ".uno:InsertPara", "", 0, Array())
end sub

'----------------------------------------------------------------------
sub copy_file_into_current_file( ByVal otherFile As String )
	dim document   as object
	dim dispatcher as object

	'get access to the document
	document   = ThisComponent.CurrentController.Frame
	dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")

	dim args2(1) as new com.sun.star.beans.PropertyValue
	args2(0).Name = "Name"
	args2(0).Value = otherFile
	args2(1).Name = "Filter"
	args2(1).Value = "writer8"	
	dispatcher.executeDispatch(document, ".uno:InsertDoc", "", 0, args2())
end sub

'-----------------------------
sub test_copy_from_other_file
	dim document   as object
	dim dispatcher as object

	Call Initialize
	Call create_empty_file("/home/mwiener/cfiles/ipc/destTmp.odt")
	Call set_current_doc("/home/mwiener/cfiles/ipc/destTmp.odt")
	document   = ThisComponent.CurrentController.Frame
	dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
'	Call Clear_current_file
	Call copy_file_into_current_file( "file:///home/mwiener/cfiles/ipc/purim_template.odt" )
	Call find_and_replace ("xxxx","משה וסיגל וינר")
	dispatcher.executeDispatch(document, ".uno:JumpToFooter", "", 0, Array())
	Call page_break
	Call copy_file_into_current_file( "file:///home/mwiener/cfiles/ipc/purim_template.odt" )
	Call find_and_replace ("xxxx","ראובן ושרון מושקין")
	Call save_current_file
	Call close_current_file
	MsgBox "Done Copy"
End Sub

'------------------------------------------------------------------------
sub test_2_copy_from_other_file
	dim document   as object
	dim dispatcher as object

	Call Initialize
	dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
	if FileExists("/home/mwiener/cfiles/ipc/destTmp.odt") = False Then
		Call set_current_doc("/home/mwiener/cfiles/ipc/destTmp.odt")
		Call Clear_current_file
	Else
		Call create_empty_file("/home/mwiener/cfiles/ipc/destTmp.odt")
		Call set_current_doc("/home/mwiener/cfiles/ipc/destTmp.odt")
	End If
	Call create_empty_file("/home/mwiener/cfiles/ipc/noteTmp.odt")
	'Copy template into temp note file
	Call set_current_doc("/home/mwiener/cfiles/ipc/destTmp.odt")
	Call copy_file_into_current_file( "file:///home/mwiener/cfiles/ipc/purim_template.odt" )
	'Replace xxxx with real name
	Call find_and_replace ("xxxx","משה וסיגל וינר")
	Call middle_align_current_file
	Call save_current_file
	Call close_current_file
	'Add the note to the Dest file
	Call set_current_doc("/home/mwiener/cfiles/ipc/destTmp.odt")
	Call copy_file_into_current_file( "file:///home/mwiener/cfiles/ipc/runner.odt" )
	Call save_current_file
	msgbox("End")
	Exit Sub
	'Add a Page Break to the Dest file
	document   = ThisComponent.CurrentController.Frame
	dispatcher.executeDispatch(document, ".uno:JumpToFooter", "", 0, Array())
	Call page_break
	Call save_current_file
	'--------- 2nd name -------------
	'Copy template into temp note file
	Call set_current_doc("/home/mwiener/cfiles/ipc/noteTmp.odt")
	Call Clear_current_file
	Call copy_file_into_current_file( "file:///home/mwiener/cfiles/ipc/purim_template.odt" )
	'Replace xxxx with real name
	Call find_and_replace ("xxxx","ראובן ושרון מושקין")
	Call save_current_file
	'Add the note to the Dest file
	Call set_current_doc("/home/mwiener/cfiles/ipc/destTmp.odt")
	Call copy_file_into_current_file( "file:///home/mwiener/cfiles/ipc/noteTmp.odt" )
	'Add a Page Break to the Dest file
	document   = ThisComponent.CurrentController.Frame
	dispatcher.executeDispatch(document, ".uno:JumpToFooter", "", 0, Array())
	Call page_break
	
	Call save_current_file
	Call close_current_file
	MsgBox "Done Copy"
End Sub

'-----------------------------------------------------------------------
sub empty_dest_file( ByVal DestFile As String )
	Dim dispatcher As Object
	Dim destDoc As Object, destFrame As Object
	Dim destFile As String, sUrl As String
	
	dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
	
	if FileExists(destFile) = True Then
		sUrl = convertToUrl( destFile )
		destDoc = StarDesktop.loadComponentFromURL(sUrl, "_blank", 0, Array())
		destFrame = destDoc.CurrentController.Frame
		'Erase text from dest file
		dispatcher.executeDispatch(destFrame, ".uno:GoToStartOfDoc", "", 0, Array())
		dispatcher.executeDispatch(destFrame, ".uno:SelectAll", "", 0, Array())
		dispatcher.executeDispatch(destFrame, ".uno:Delete", "", 0, Array())
		dispatcher.executeDispatch(destFrame, ".uno:Save", "", 0, Array())
		destFrame.Close(False)
	End If
End Sub

'-----------------------------------------------------------------------
sub test_make_notes
	Dim destFile As String, templateFile As String
	Dim names As String, giver As String, families As String
	Dim records(1 To 4) As String
	
	destFile = "/home/mwiener/cfiles/ipc/destTmp.odt"
	templateFile = "/home/mwiener/cfiles/ipc/purim_template.odt"
	records(1) = "משה וסיגל וינר"
	records(2) = "ראובן ושרון מושקין"
	records(3) = "שלמה ונילי וינר"
	records(4) = "ניסים ורחל אוחיון"
	Call empty_dest_file( destFile )
	names = records(1) & "," & records(2) & "," & records(3) & "," & records(4) & Chr(10)
	parseNamesList( names, giver, families )
	Call Add_Note(destFile, templateFile, giver, families)
	
	names = records(2) & "," & records(3) & "," & records(4) & "," & records(1) & Chr(10)
	parseNamesList( names, giver, families )
	Call Add_Note(destFile, templateFile, giver, families)
	
	names = records(3) & "," & records(4) & "," & records(1) & "," & records(2) & Chr(10)
	parseNamesList( names, giver, families )
	Call Add_Note(destFile, templateFile, giver, families)
	
	names = records(4) & "," & records(1) & "," & records(2) & "," & records(3) & Chr(10)
	parseNamesList( names, giver, families )
	Call Add_Note(destFile, templateFile, giver, families)
	MsgBox "Done test_make_notes"
End Sub

'-----------------------------------------------------------------------
Sub Add_Note(ByVal DestFile As String, ByVal templateFile As String, _
				ByVal FamilyName As String, ByVal receivers As String )
	dim destDoc As Object, templateDoc As Object, noteDoc As Object
	dim dispatcher as object, dispatcher2 As Object
	Dim destFrame As Object, templateFrame As Object, noteFrame As Object
	Dim noteFile As String
	Dim sUrl As String 
	dim args(1) as new com.sun.star.beans.PropertyValue
	dim args2(1) as new com.sun.star.beans.PropertyValue
	dim args3(0) as new com.sun.star.beans.PropertyValue
	
	noteFile = "/home/mwiener/cfiles/ipc/noteTmp.odt"
	dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
	dispatcher2 = createUnoService("com.sun.star.frame.DispatchHelper")
	'Create dest file
	if FileExists(destFile) = False Then
		'Call LoadNewWriterDoc(destFile)
		FileCopy (templateFile, destFile)
		sUrl = convertToUrl( destFile )
		destDoc = StarDesktop.loadComponentFromURL(sUrl, "_blank", 0, Array())
		destFrame = destDoc.CurrentController.Frame
		'Erase text from dest file
		dispatcher.executeDispatch(destFrame, ".uno:GoToStartOfDoc", "", 0, Array())
		dispatcher.executeDispatch(destFrame, ".uno:SelectAll", "", 0, Array())
		dispatcher.executeDispatch(destFrame, ".uno:Delete", "", 0, Array())
		dispatcher.executeDispatch(destFrame, ".uno:Save", "", 0, Array())
	Else
		sUrl = convertToUrl( destFile )
		destDoc = StarDesktop.loadComponentFromURL(sUrl, "_blank", 0, Array())
		destFrame = destDoc.CurrentController.Frame
	End If
	
	'Set destfile as current doc		
	args(0).Name = "Count"
	args(0).Value = 1
	args(1).Name = "Select"
	args(1).Value = false
	dispatcher.executeDispatch(destFrame, ".uno:GoDown", "", 0, args())
	dispatcher.executeDispatch(destFrame, ".uno:JumpToHeader", "", 0, Array())
	dispatcher.executeDispatch(destFrame, ".uno:Escape", "", 0, Array())
	dispatcher.executeDispatch(destFrame, ".uno:JumpToFooter", "", 0, Array())
	dispatcher.executeDispatch(destFrame, ".uno:GoToStartOfDoc", "", 0, Array())	
	'Create note file
	if FileExists(noteFile) = False Then
		Call FileCopy (templateFile, noteFile)
	End If

	'Set notefile as current doc
	sUrl = convertToUrl( noteFile )
	noteDoc = stardesktop.LoadComponentFromURL(sUrl, "_blank", 0, Array())
	noteFrame = noteDoc.CurrentController.Frame		
	args(0).Name = "Count"
	args(0).Value = 1
	args(1).Name = "Select"
	args(1).Value = false
	dispatcher2.executeDispatch(noteFrame, ".uno:GoDown", "", 0, args())
	dispatcher2.executeDispatch(noteFrame, ".uno:JumpToHeader", "", 0, Array())
	dispatcher2.executeDispatch(noteFrame, ".uno:Escape", "", 0, Array())
	dispatcher2.executeDispatch(noteFrame, ".uno:JumpToFooter", "", 0, Array())
	dispatcher2.executeDispatch(noteFrame, ".uno:GoToStartOfDoc", "", 0, Array())
	
	'Clear the text in notefile
	dispatcher2.executeDispatch(noteFrame, ".uno:SelectAll", "", 0, Array())
	dispatcher2.executeDispatch(noteFrame, ".uno:Delete", "", 0, Array())
	
	'Copy template into note file
	args2(0).Name = "Name"
	args2(0).Value = convertToUrl( templateFile )
	args2(1).Name = "Filter"
	args2(1).Value = "writer8"	
	dispatcher2.executeDispatch(noteFrame, ".uno:InsertDoc", "", 0, args2())
	'Replace xxxx with real name
	Call find_and_replace ("xxxx", FamilyName)
	'Replace yyyy with receivers list
	Call find_and_replace ("yyyy", receivers)
	'Save notefile
	dispatcher2.executeDispatch(noteFrame, ".uno:Save", "", 0, Array())
	'Close notefile	
	noteFrame.Close(False)
	'Set destfile as current doc	
	args(0).Name = "Count"
	args(0).Value = 1
	args(1).Name = "Select"
	args(1).Value = false
	dispatcher.executeDispatch(destFrame, ".uno:GoDown", "", 0, args())
	dispatcher.executeDispatch(destFrame, ".uno:JumpToHeader", "", 0, Array())
	dispatcher.executeDispatch(destFrame, ".uno:Escape", "", 0, Array())
	dispatcher.executeDispatch(destFrame, ".uno:JumpToFooter", "", 0, Array())
	dispatcher.executeDispatch(destFrame, ".uno:GoToStartOfDoc", "", 0, Array())
	'Add notefile to tail of destfile
	dispatcher.executeDispatch(destFrame, ".uno:GoToEndOfDoc", "", 0, Array())
	args2(0).Name = "Name"
	args2(0).Value = convertToUrl( noteFile )
	args2(1).Name = "Filter"
	args2(1).Value = "writer8"	
	dispatcher.executeDispatch(destFrame, ".uno:InsertDoc", "", 0, args2())
	'Add Page Break after the note
	dispatcher.executeDispatch(destFrame, ".uno:InsertPagebreak", "", 0, Array())
	'Middle align destfile
	dispatcher.executeDispatch(destFrame, ".uno:SelectAll", "", 0, Array())
	args2(0).Name = "CenterPara"
	args2(0).Value = true
	dispatcher.executeDispatch(destFrame, ".uno:CenterPara", "", 0, args2())	
	args3(0).Name = "ParaRightToLeft"
	args3(0).Value = true
	dispatcher.executeDispatch(destFrame, ".uno:ParaRightToLeft", "", 0, args3())

	'Save dest
	dispatcher.executeDispatch(destFrame, ".uno:Save", "", 0, Array())
	'Close destFile	
	destFrame.Close(False)	
End Sub

sub find_and_replace (byval old_text as String, byval new_text as string)
rem ----------------------------------------------------------------------
rem define variables
dim document   as object
dim dispatcher as object

document   = ThisComponent.CurrentController.Frame
dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")

dim args1(21) as new com.sun.star.beans.PropertyValue
args1(0).Name = "SearchItem.StyleFamily"
args1(0).Value = 2
args1(1).Name = "SearchItem.CellType"
args1(1).Value = 0
args1(2).Name = "SearchItem.RowDirection"
args1(2).Value = true
args1(3).Name = "SearchItem.AllTables"
args1(3).Value = false
args1(4).Name = "SearchItem.SearchFiltered"
args1(4).Value = false
args1(5).Name = "SearchItem.Backward"
args1(5).Value = false
args1(6).Name = "SearchItem.Pattern"
args1(6).Value = false
args1(7).Name = "SearchItem.Content"
args1(7).Value = false
args1(8).Name = "SearchItem.AsianOptions"
args1(8).Value = false
args1(9).Name = "SearchItem.AlgorithmType"
args1(9).Value = 0
args1(10).Name = "SearchItem.SearchFlags"
args1(10).Value = 65536
args1(11).Name = "SearchItem.SearchString"
args1(11).Value = old_text
args1(12).Name = "SearchItem.ReplaceString"
args1(12).Value = new_text
args1(13).Name = "SearchItem.Locale"
args1(13).Value = 255
args1(14).Name = "SearchItem.ChangedChars"
args1(14).Value = 2
args1(15).Name = "SearchItem.DeletedChars"
args1(15).Value = 2
args1(16).Name = "SearchItem.InsertedChars"
args1(16).Value = 2
args1(17).Name = "SearchItem.TransliterateFlags"
args1(17).Value = 1024
args1(18).Name = "SearchItem.Command"
args1(18).Value = 0
args1(19).Name = "SearchItem.SearchFormatted"
args1(19).Value = false
args1(20).Name = "SearchItem.AlgorithmType2"
args1(20).Value = 1
args1(21).Name = "Quiet"
args1(21).Value = true

dispatcher.executeDispatch(document, ".uno:ExecuteSearch", "", 0, args1())

dim args2(21) as new com.sun.star.beans.PropertyValue
args2(0).Name = "SearchItem.StyleFamily"
args2(0).Value = 2
args2(1).Name = "SearchItem.CellType"
args2(1).Value = 0
args2(2).Name = "SearchItem.RowDirection"
args2(2).Value = true
args2(3).Name = "SearchItem.AllTables"
args2(3).Value = false
args2(4).Name = "SearchItem.SearchFiltered"
args2(4).Value = false
args2(5).Name = "SearchItem.Backward"
args2(5).Value = false
args2(6).Name = "SearchItem.Pattern"
args2(6).Value = false
args2(7).Name = "SearchItem.Content"
args2(7).Value = false
args2(8).Name = "SearchItem.AsianOptions"
args2(8).Value = false
args2(9).Name = "SearchItem.AlgorithmType"
args2(9).Value = 0
args2(10).Name = "SearchItem.SearchFlags"
args2(10).Value = 65536
args2(11).Name = "SearchItem.SearchString"
args2(11).Value = old_text
args2(12).Name = "SearchItem.ReplaceString"
args2(12).Value = new_text
args2(13).Name = "SearchItem.Locale"
args2(13).Value = 255
args2(14).Name = "SearchItem.ChangedChars"
args2(14).Value = 2
args2(15).Name = "SearchItem.DeletedChars"
args2(15).Value = 2
args2(16).Name = "SearchItem.InsertedChars"
args2(16).Value = 2
args2(17).Name = "SearchItem.TransliterateFlags"
args2(17).Value = 1024
args2(18).Name = "SearchItem.Command"
args2(18).Value = 2
args2(19).Name = "SearchItem.SearchFormatted"
args2(19).Value = false
args2(20).Name = "SearchItem.AlgorithmType2"
args2(20).Value = 1
args2(21).Name = "Quiet"
args2(21).Value = true

dispatcher.executeDispatch(document, ".uno:ExecuteSearch", "", 0, args2())


end sub

REM --------------------------------------------------------
sub test
 find_and_replace ("XXX","Mr. Simpson")
end sub


sub copy_internal
rem ----------------------------------------------------------------------
rem define variables
dim document   as object
dim dispatcher as object

  select_all
rem ----------------------------------------------------------------------
rem get access to the document
document   = ThisComponent.CurrentController.Frame
dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:Copy", "", 0, Array())

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:Paste", "", 0, Array())


end sub


sub select_all
rem ----------------------------------------------------------------------
rem define variables
dim document   as object
dim dispatcher as object
rem ----------------------------------------------------------------------
rem get access to the document
document   = ThisComponent.CurrentController.Frame
dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:SelectAll", "", 0, Array())


end sub



sub select_all_and_copy
rem ----------------------------------------------------------------------
rem define variables
dim document   as object
dim dispatcher as object
rem ----------------------------------------------------------------------
rem get access to the document
document   = ThisComponent.CurrentController.Frame
dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:SelectAll", "", 0, Array())

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:Copy", "", 0, Array())

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:Paste", "", 0, Array())

rem ----------------------------------------------------------------------
dispatcher.executeDispatch(document, ".uno:Paste", "", 0, Array())
end sub

Sub test_parseNamesList
	Dim names As String, giver As String, families As String
	
	names = "משה וינר,אפרים הנגבי,שלמה וינר"
	Call parseNamesList(names, giver, families)
	MsgBox("השולח:" & giver & chr(10) & "רשימת המשפחות" & chr(10) & families
End Sub

'-----------------------------------------------------------------------
sub parseNamesList( ByVal names As String, ByRef giver As String, ByRef families As String )
	Dim giverLen As Integer, familyLen As Integer, namesLen As Integer
	Dim familyStart As Integer, familyEnd As Integer
	
	families = ""
	namesLen = Len(names)
	giverLen = InStr(1,names,",") - 1
	giver = left(names, giverLen)
	familyStart = giverLen + 2
	While familyStart < namesLen
		familyEnd = InStr(familyStart, names, ",")
		if familyEnd <= familyStart then
			familyEnd = namesLen + 1
		End If
		familyLen = familyEnd - familyStart
		families = families & Mid(names, familyStart, familyLen) & chr(10)  
		familyStart = FamilyEnd + 1 
	Wend
end sub


'----------------------------------------------------------------------
sub Clear_current_file
dim document   as object
dim dispatcher as object

document   = ThisComponent.CurrentController.Frame
dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
dispatcher.executeDispatch(document, ".uno:SelectAll", "", 0, Array())
dispatcher.executeDispatch(document, ".uno:Delete", "", 0, Array())
end sub

'---------------------------------------------------------------------
Sub save_current_file
	Dim document   As Object
	Dim dispatcher As Object
	
	document   = ThisComponent.CurrentController.Frame
	dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
	dispatcher.executeDispatch(document, ".uno:Save", "", 0, Array())
End  Sub

'----------------------------------------------------------------------
Sub close_current_file
	Dim oDocFrame As Object
	
	oDocFrame = ThisComponent.CurrentController.Frame
	If isNull(oDocFrame) = False Then	
		oDocFrame.Close(False)
	End If
End Sub

'----------------------------------------------------------------------
sub test2
	Call create_empty_file("/home/mwiener/cfiles/ipc/destTmp.odt")
	Call set_current_doc("/home/mwiener/cfiles/ipc/purim_template.odt")
	Call copy_current_file_into_another("/home/mwiener/cfiles/ipc/destTmp.odt")
	Call close_current_file
	Call set_current_doc("/home/mwiener/cfiles/ipc/destTmp.odt")
	Call middle_align_current_file
	Call save_current_file
	Call close_current_file
	MsgBox("Done test2")
end sub

'----------------------------------------------------------------------
sub test3
	Dim oDocFrame As Object
	
	Call Initialize
	Call set_current_doc("/home/mwiener/cfiles/ipc/purim_template.odt")
	MsgBox("After set_current_doc")
	Call copy_current_file_into_another("/home/mwiener/cfiles/ipc/destTmp.odt")
	MsgBox ("After copy_current_file_into_another")
	Call close_current_file
	MsgBox "After close_current_file"
	Call set_current_doc("/home/mwiener/cfiles/ipc/purim_template.odt")
	Call close_current_file
	MsgBox("Done test3")
end sub

'----------------------------------------------------------------------
sub Middle_rightToLeft
	dim document   as object
	dim dispatcher as object
	dim args2(0) as new com.sun.star.beans.PropertyValue
	dim args3(0) as new com.sun.star.beans.PropertyValue
	
	document   = ThisComponent.CurrentController.Frame
	dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
	dispatcher.executeDispatch(document, ".uno:SelectAll", "", 0, Array())
	args2(0).Name = "ParaRightToLeft"
	args2(0).Value = true
	dispatcher.executeDispatch(document, ".uno:ParaRightToLeft", "", 0, args2())
	
	args3(0).Name = "CenterPara"
	args3(0).Value = true
	dispatcher.executeDispatch(document, ".uno:CenterPara", "", 0, args3())
end sub

'------------------------------------------------------------------------
Sub InsertBackgroundImage
Dim oStyleFamilies As Variant
Dim oPageStyles As Variant
Dim oFirstPageStyle As Variant
Dim oBitmaps
dim document   as object
dim dispatcher as object
dim args1(1) as new com.sun.star.beans.PropertyValue

document   = ThisComponent.CurrentController.Frame
dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
REM 1. Insert image into document (folder Pictures in ZIP)
    oBitmaps = ThisComponent.createInstance( "com.sun.star.drawing.BitmapTable" )
REM First parameter - internal name of image, second - real path to file on disc
    oBitmaps.insertByName( "newBkg", ConvertToURL("/home/mwiener/cfiles/ipc/scroll.jpg") )
REM 2. Get stile of title page
    oStyleFamilies = ThisComponent.getStyleFamilies()
    oPageStyles = oStyleFamilies.getByName("PageStyles")
    oFirstPageStyle = oPageStyles.getByName("First Page")
REM 3. Set URL of internal image as BackGraphicURL
    oFirstPageStyle.BackGraphicURL = oBitmaps.getByName("newBkg")
    oFirstPageStyle.BackGraphicLocation = com.sun.star.style.GraphicLocation.AREA
REM Now just apply style "First Page" to your document
	args1(0).Name = "Template"
	args1(0).Value = "First Page"
	args1(1).Name = "Family"
	args1(1).Value = 8
	dispatcher.executeDispatch(document, ".uno:StyleApply", "", 0, args1())
End Sub

sub Style
rem ----------------------------------------------------------------------
rem define variables
dim document   as object
dim dispatcher as object
rem ----------------------------------------------------------------------
rem get access to the document
document   = ThisComponent.CurrentController.Frame
dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")

rem ----------------------------------------------------------------------
dim args1(1) as new com.sun.star.beans.PropertyValue
args1(0).Name = "Template"
args1(0).Value = "HTML"
args1(1).Name = "Family"
args1(1).Value = 8

dispatcher.executeDispatch(document, ".uno:StyleApply", "", 0, args1())


end sub
