<html><head>
<title>vAdmin Portal</title>
<script type='text/javascript' src='js/jquery.js'></script>
<script type='text/javascript' src='js/jwysiwyg/jquery.wysiwyg.js'></script>
<script type='text/javascript' src='js/jwysiwyg/controls/wysiwyg.image.js'></script>
<script type='text/javascript' src='js/jwysiwyg/controls/wysiwyg.link.js'></script>
<script type='text/javascript' src='js/jwysiwyg/controls/wysiwyg.table.js'></script>
<script type='text/javascript' src='js/jwysiwyg/controls/wysiwyg.colorpicker.js'></script>

<script type='text/javascript'>
var data; // each page and their content
var messageTimer; // timeout handle for messages so we don't hide the wrong message

function displayMessage(text){
  $('#message').show();
  $('#message').html(text);
  if (messageTimer != undefined) {
    clearTimeout(messageTimer);
  }
  messageTimer = setTimeout(function() { $('#message').hide(); }, 5000);
}

function page_list_to_html(parentObj, page_data) {

  for(var i = 0; i < page_data.length; i = i+1)
  {
    var page_title = page_data[i].title;
    var obj =  $("<div class='page-select-radio'></div>")
      .append($("<input></input>")
        .attr({type:'radio',name:'page-select', id:page_title})
        .click({content:page_data[i].content, title:page_data[i].title}, function(event) {
          $('#edit-area').show();
          $('#editor').wysiwyg('clear');
          $('#editor').wysiwyg('setContent',event.data.content)
        })
      )
      .append($('<label></label>').attr({for:page_title}).html(page_title));
    parentObj.append(obj); 
  }
}

function add_new_page() {
   var obj =  $("<div class='page-select-radio'></div>")
    .append($("<input></input>")
      .attr({type:'radio',name:'page-select', id:'new_page'})
      .click(function(event) {
        $('#edit-area').show();
        $('#editor').wysiwyg('clear');
        $('#editor').wysiwyg('setContent','');
      })
    )
    .append($('<input></input>').attr({type:'text',name:'page-select'}));

  return obj;
}

function get_last_modified(pages_data, page_title) {
  for (var i = 0; i < pages_data.length;  i += 1) 
  { 
    if (pages_data[i].title == page_title)
      return pages_data[i].last_modified;
  }
  return false;
}

function submit_changes() {

  if ($("input[name='page-select']:checked")[0] == undefined) {
    displayMessage('Please select a page to edit.');
    return;
  }
  var mod_page_title = $("input[name='page-select']:checked")[0].id;

  // pull up the last_modified timestamp of the correct page
  var last_mod_time = get_last_modified(data.data, mod_page_title);

  $.post('ajax/submit_changes.php',
    {title: mod_page_title,
     content: $("#editor").val(),
     pass: $('#password').val(),
     last_modified: last_mod_time,},
    function(result){
      var data = eval('('+result+')');
      if (data.error != 0) {
        displayMessage(data.error_msg);
      } else {
        $('#edit-area').hide();
        displayMessage("Changes saved!");
        setTimeout('get_page_list();', 2000);
      }
    });
}

function append_auth(parentObj, callback) {
  parentObj
    .append($("<label></label>").attr({for:'password'}).html("password:").wrap("<h2></h2>"))
    .append($("<input></input>").attr({type:'password', id:'password'}))
    .append($("<input></input>").attr({type:'submit'}).click(callback));
}

function get_page_list() {
  $.ajax({
    type: "POST",
    url: "ajax/get_page_list.php",
    data: "pass=" + $('#password').val(),
    success: function(datatxt) {
      data = eval('(' + datatxt + ')');
      //console.log(data);
      if (data.error != 0) {
        displayMessage(data.error_msg);
      } else {
        $('#content').html("");
        displayMessage('Select one of the static pages to edit');
        page_list_to_html($('#content'), data.data);
        append_auth($('#content'), submit_changes);
      }
    }
  });
}

function toggle_editor() {
  if ($('#editor').is(':visible')) { 
    // enabling wysiwyg (update content of wysiwyg window)
    $('#editor').wysiwyg('setContent', $('#editor').val());
    
    $('#editor').hide();
    $('.wysiwyg').show();
  } else {
    $('.wysiwyg').hide();
    $('#editor').show();
  }
}

$('body').ready(function(){
  $('#editor').wysiwyg({
    controls:{
      code: {visible: false},
      colorpicker: {visible: true},
      section: {
        visible: true,
        groupIndex:1,
        icon: 'js/jwysiwyg/help/tests/images/quote02.gif',
        tooltip: 'Wrap as section',
        tags: ['section'],
        exec: function () {
          var range = this.getInternalRange(),
              common  = range.commonAncestorContainer,
              section = this.dom.getElement("section");

          // if a text node is selected, we want to make the wrap the whole element, not just some text
          if (common.nodeType === 3) {
            common = common.parentNode;
          }

          if (section && $(section).hasClass("section")) {
            $(common).unwrap();
          }
          else {
            if ("body" !== common.nodeName.toLowerCase()) {
              $(common).wrap("<div class='section' />");
            }
          }
        },
        callback: function (event, Wysiwyg) {
          console.log("wrapping text in section");
        }
      },
    },
    css: '../styles/basic.css',
  });
  $('#edit-area').hide();

  $('#content').html('');
  append_auth($('#content'), get_page_list);
});

</script>
<link rel="stylesheet" type="text/css" href="js/jwysiwyg/jquery.wysiwyg.css" />
<style type="text/css">
#message {
  font-size: 16px;
  background: #aaa;
  border: 2px black solid;
  display:none;
}
#content{
  margin: 10px auto;
  width: 300px;
  text-align: left;
}
</style>
</head>
<body style='text-align:center'>

<h1>Stanford Viennese Steering WebAdmin Portal</h1>
<div id='message'></div>
<table style='margin: 0 auto'>
<tr style='vertical-align: top'>
<td><div id='content'>
</div><!-- end content --></td>
<td><div id='edit-area' style='width:710px; margin:0 auto;'>
<a href='#' onclick='toggle_editor()'>Toggle editor</a>
<textarea id='editor' cols=80 rows=50></textarea>
</div></td>
</table>
</body></html>

