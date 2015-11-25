import httplib
import re
from HTMLParser import HTMLParser
import json

# This will scrape radar.weather.gov to find the coordinates of the various
# Doppler stations that are available

class StationListParser(HTMLParser):
    record = False
    found = []
    def handle_starttag(self, tag, attrs):
        if tag == "select" and attrs[1][1] == 'bysite':
            print "Found a select, attrs", attrs
            StationListParser.record = True
    def handle_endtag(self, tag):
        if tag == "select" and StationListParser.record:
            StationListParser.record = False
    def handle_data(self, data):
        if StationListParser.record:
            StationListParser.found.append(data)


conn = httplib.HTTPConnection("www.srh.noaa.gov")
req = conn.request("GET", "/jetstream/doppler/ridge_download.htm")
resp = conn.getresponse()
stations_dat = resp.read()
stations_parser = StationListParser()
stations_parser.feed(stations_dat)
prefiltered = stations_parser.found
stations = map(lambda y: y[0:3], filter(lambda x: x != "\r\n", prefiltered))

# Now that we know all the stations, let's find their coordinates!
station_coords = {}
conn = httplib.HTTPConnection("radar.weather.gov")
for station in stations:
    conn.request("GET", "/radar.php?rid=" + station, None, {"User-Agent":
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like" + \
        "Gecko) Chrome/46.0.2490.86 Safari/537.36"})
    resp = conn.getresponse()
    station_dat = resp.read()
    # I'm doing it. Deal with it.
    xllpat = re.compile("^xllcorner = ([0-9.-]+);.*$", flags=re.MULTILINE)
    yllpat = re.compile("^yllcorner = ([0-9.-]+);.*$", flags=re.MULTILINE)
    xypat = re.compile("^xyperpixel = ([0-9.-]+);.*$", flags=re.MULTILINE)
    try:
        xcorner = float(xllpat.search(station_dat).group(1))
        ycorner = float(yllpat.search(station_dat).group(1))
        xyperpixel = float(xypat.search(station_dat).group(1))
    except: 
        print "Skipping", station
        continue
    # The following is stolen from their javascript
    lat = xcorner-(275*xyperpixel)
    lon = ycorner+(300*xyperpixel)
    station_coords[station] = (lat, lon)

print station_coords
fh = open("station_coordinates.json", "w")
fh.write(json.dumps(station_coords))
fh.close()
