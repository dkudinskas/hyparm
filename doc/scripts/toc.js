function buildTOC()
{
  var container = new Element('div', { 'id': 'tocList' });
  var toc = $('tocText').get('text').split('\n');
  var firstLevelList = new Element('ul');
  var secondLevelList = null;
  var linkPrefix = null;
  for (var i = 0; i < toc.length; ++i)
  {
    var title = toc[i].clean();
    if (title.length < 1)
    {
      continue;
    }
    if (title.test(/^\* /))
    {
      if (secondLevelList)
      {
        title = title.substr(2);
        var filename = linkPrefix + textToFilename(title.toLowerCase().replace(/[^a-z]+/g, '-')) + '.html';
        var listItem = new Element('li');
        listItem.appendChild(new Element('a', { 'href': filename, 'target': 'content', 'text': title }));
        secondLevelList.appendChild(listItem);
      }
    }
    else
    {
      var listItem = new Element('li', {'text': title});
      listItem.appendChild(secondLevelList = new Element('ul'));
      firstLevelList.appendChild(listItem);
      linkPrefix = textToFilename(title) + '/';
    }
  }
  container.appendChild(firstLevelList);
  document.body.appendChild(container);
}

function textToFilename(text)
{
 return text.toLowerCase().replace(/[^a-z]+/g, '-');
}

window.addEvent('domready', buildTOC);
