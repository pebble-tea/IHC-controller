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
