# Aggregate the web page info spread across several page files in a dir into page link file and page content file
# usage: python create_page_info.py $dir

import sys,os

def parse_page(fpath):
    """Open a page info file and parse it into dict"""
    f = open(fpath, 'r')
    content_begin = False
    d = {}
    f_link = []
    for line in f:
        if line == '---------------------\n':
            content_begin = True
            continue
        if content_begin == False:
            page_id = int( line.strip().replace('page', '') )
            if page_id not in f_link:
                f_link.append(page_id)
        else:
            d['content'] = line.strip()
    d['f_link'] = f_link
    d['r_link'] = []
    f.close()
    return d


def store_link_file(fname, pages, link_type):
    """Store the link info into file"""
    f = open(fname, 'w+')
    for page in pages:
        link = map(str, page[link_type])
        link_info = ' '.join(link)
        f.write(link_info + '\n')
    f.close()

if __name__ == '__main__':

    #Set up dir path
    dir_path = sys.argv[1]
    if dir_path[-1] != '/':
        dir_path = dir_path + '/'

    #Get the dir f_list sorted according to page number
    f_list = [ os.path.join(dir_path, f) for f in os.listdir(dir_path) ]
    id_list = [ int( fpath.replace(dir_path + 'page', '') ) for fpath in f_list ]
    f_id_table = dict(zip(id_list, f_list))
    f_list = [ fpath for f_id,fpath in sorted(f_id_table.iteritems()) ]
    
    #Open and parse every page info file
    print "Constructing forward links and reading in page content ..."
    pages = []
    for fpath in f_list:
        pages.append(parse_page(fpath))
    
    #Construct reverse link from forward link for each pages
    print "Constructing reverse links ..."
    for i,page in enumerate(pages):
        for page_id in page['f_link']:
            if i not in pages[page_id]['r_link']:
                pages[page_id]['r_link'].append(i)
    
    #Store the r_link,r_link into files
    print "Storing f_link and r_link into files ..."
    store_link_file('f_link.txt', pages, 'f_link')
    store_link_file('r_link.txt', pages, 'r_link')

    #Store the page content into file
    print "Storing page content into file ..."
    f = open('page_content.txt', 'w+')
    for page in pages:
        f.write(page['content'] + '\n')
    f.close()
