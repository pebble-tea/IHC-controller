$("#ON").click(()=>
{
  $.get('/api?state=255', (data)=>
  {
    $( ".result" ).html( data );
    console.log("Load was performed.");
  });
});

$("#OFF").click(()=>
{
  $.get('/api?state=0', (data)=>
  {
    $( ".result" ).html( data );
    console.log("Load was performed.");
  });
});

$('#points').on('input', function () {
    $(this).trigger('change');
});

$("#points").change(function(){
    var newval=$(this).val();
    $.get('/api?state='+newval, (data)=>
    {
      $( ".result" ).html( data );
      console.log(data);
    });
  });
