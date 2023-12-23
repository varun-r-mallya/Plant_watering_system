var SS = SpreadsheetApp.openById('1zIdt5UI3NgEUyF7FMaQC0UFEQJ902NfOVyP7DmJWO28');
var str = "";
var sheet = SS.getSheetByName('Sheet1');

function doGet(e){
  
    var read = e.parameter.read;
    
    if (read !== undefined){
      return ContentService.createTextOutput(sheet.getRange(read).getValue());
    }
    
    return ContentService.createTextOutput("No value passed as argument to script Url.");
  }
  
