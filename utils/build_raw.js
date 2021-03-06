const fs = require('fs');
const process = require('process');
const { createCanvas, loadImage } = require('canvas');
// var w = 296;
// var h = 128;
var sw = 296;
var sh = 128;
var w = 128;
var h = 296;
var curPal = [[0,0,0],[255,255,255],[220,180,0]];
var total_pages = 0;


// var image_name = 'test2';
// var file = image_name + '.png';

var book_path = process.argv[2];
if (!book_path) {
  console.log("need book path");
  return;
}

var book_src = book_path + 'src/';
var book_data = book_path + 'data/';
console.log(book_src);

fs.readdir(book_src, (err, files) => {
    if (err) throw err;
    console.log(files);
    // for (var i=0; i<files.length; i++) {
    total_pages = files.length;

    files.forEach(function(file) {
      var file_path = book_src + file;
      var image_name = file.split('.')[0];

      console.log('dd ' + file_path);

      loadImage(file_path).then((image) => {
        console.log(file_path);
        var source = createCanvas(w, h);
        var canvas = createCanvas(w, h);
        var ctx=source.getContext('2d');
        ctx.translate(image.height/2, image.width/2);
        ctx.rotate(270*Math.PI/180);
        ctx.drawImage(image, -image.width/2, -image.height/2);
        // source.getContext('2d').drawImage(image, 0, 0, w, h);

        var pSrc = ctx.getImageData(0, 0, w, h);
        var pDst = processImage(pSrc);
        canvas.getContext('2d').putImageData(pDst, 0, 0);

        var out = fs.createWriteStream(book_data + image_name + '_r.png');
        var stream = canvas.createPNGStream();
        stream.pipe(out);

        var p = pDst;
        var a = new Array(w*h);
        var i = 0;
        for(var y=0;y<h;y++)for(var x=0;x<w;x++,i++)a[i]=getVal(p,i<<2);
        var html = "TOTAL"+total_pages+'\n'+makeRawData(a);
        fs.writeFile(book_data + image_name + '.html', html, 'utf8', (err) => {
            if (err) throw err;
            console.log('raw data saved: ' + image_name);
        });

      });


    });
});



function getVal(p, i){
  if((p.data[i]==0x00) && (p.data[i+1]==0x00))return 0;
  if((p.data[i]==0xFF) && (p.data[i+1]==0xFF))return 1;
  if((p.data[i]==0x7F) && (p.data[i+1]==0x7F))return 2;
  return 3;
}


function setVal(p,i,c){
  p.data[i]=curPal[c][0];
  p.data[i+1]=curPal[c][1];
  p.data[i+2]=curPal[c][2];
  p.data[i+3]=255;
}


function addVal(c,r,g,b,k){
  return[c[0]+(r*k)/32,c[1]+(g*k)/32,c[2]+(b*k)/32];
}


function getErr(r,g,b,stdCol){
  r-=stdCol[0];
  g-=stdCol[1];
  b-=stdCol[2];
  return r*r + g*g + b*b;
}


function getNear(r,g,b){
  var ind=0;
  var err=getErr(r,g,b,curPal[0]);
  for (var i=1;i<curPal.length;i++)
  {
    var cur=getErr(r,g,b,curPal[i]);
    if (cur<err){err=cur;ind=i;}
  }
  return ind;
}


function processImage(pSrc) {
  var canvas = createCanvas(w, h);
  var pDst = canvas.getContext('2d').getImageData(0, 0, w, h);
  var dX = 0;
  var dY = 0;
  var dW = w;
  var dH = h;
  var sW = w;
  var sH = h;
  var aInd=0;
  var bInd=1;
  var index = 0;
  var errArr=new Array(2);
  errArr[0]=new Array(dW);
  errArr[1]=new Array(dW);

  for (var i=0;i<dW;i++)
    errArr[bInd][i]=[0,0,0];
  for (var j=0;j<dH;j++){
  var y=dY+j;

  if ((y<0)||(y>=sH)){
    for (var i=0;i<dW;i++,index+=4)setVal(pDst,index,(i+j)%2==0?1:0);
    continue;
  }

  aInd=((bInd=aInd)+1)&1;

  for (var i=0;i<dW;i++)errArr[bInd][i]=[0,0,0];
  for (var i=0;i<dW;i++){
    var x=dX+i;
    if ((x<0)||(x>=sW)){
      setVal(pDst,index,(i+j)%2==0?1:0);
      index+=4;
      continue;
    }
    var pos=(y*sW+x)*4;
    var old=errArr[aInd][i];
    var r=pSrc.data[pos  ]+old[0];
    var g=pSrc.data[pos+1]+old[1];
    var b=pSrc.data[pos+2]+old[2];
    var colVal = curPal[getNear(r,g,b)];
    pDst.data[index++]=colVal[0];
    pDst.data[index++]=colVal[1];
    pDst.data[index++]=colVal[2];
    pDst.data[index++]=255;
    r=(r-colVal[0]);
    g=(g-colVal[1]);
    b=(b-colVal[2]);

    if (i==0){
      errArr[bInd][i  ]=addVal(errArr[bInd][i  ],r,g,b,7.0);
      errArr[bInd][i+1]=addVal(errArr[bInd][i+1],r,g,b,2.0);
      errArr[aInd][i+1]=addVal(errArr[aInd][i+1],r,g,b,7.0);
    }else if (i==dW-1){
      errArr[bInd][i-1]=addVal(errArr[bInd][i-1],r,g,b,7.0);
      errArr[bInd][i  ]=addVal(errArr[bInd][i  ],r,g,b,9.0);
    }else{
      errArr[bInd][i-1]=addVal(errArr[bInd][i-1],r,g,b,3.0);
      errArr[bInd][i  ]=addVal(errArr[bInd][i  ],r,g,b,5.0);
      errArr[bInd][i+1]=addVal(errArr[bInd][i+1],r,g,b,1.0);
      errArr[aInd][i+1]=addVal(errArr[aInd][i+1],r,g,b,7.0);
    }
  }
  }
  return pDst;
}


function byteToStr(v){return String.fromCharCode((v & 0xF) + 97, ((v >> 4) & 0xF) + 97);}


function makeRawData(a) {
  var html = '<pre>\n'

  // while((pxInd<a.length)&&(rqMsg.length<1500)) {
  c = 0;
  var rqMsg = '';
  var pxInd = 0;
  while(pxInd < a.length) {
    var v=0;
    for (var i=0;i<8;i++) {
      if((pxInd<a.length)&&(a[pxInd]!=c))v|=(128>>i);
      pxInd++;
    }
    rqMsg += byteToStr(v);

    if (rqMsg.length >= 1500) {
      html += 'LOAD' + rqMsg + '\n';
      rqMsg = '';
    }
  }
  html += 'LOAD' + rqMsg + '\n';
  console.log(rqMsg);

  c = 3;
  var rqMsg = '';
  var pxInd = 0;
  while(pxInd < a.length) {
    var v=0;
    for (var i=0;i<8;i++) {
      if((pxInd<a.length)&&(a[pxInd]!=c))v|=(128>>i);
      pxInd++;
    }
    rqMsg += byteToStr(v);

    if (rqMsg.length >= 1500) {
      html += 'LOAD' + rqMsg + '\n';
      rqMsg = '';
    }
  }
  html += 'LOAD' + rqMsg + '\n';

  return html+'</pre>';

}
