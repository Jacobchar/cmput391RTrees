# cmput391RTrees
Assignment 2 of CMPUT 391 in which we are to implement and test an R-Tree for recording and manipulating spatial data in two dimensions. This will be used to evaluate the cost of different 'nearest neighbour' queries using different access methods.

Q0 "Magic Numbers"
For our data conversion in question 0 of the assignment we chose to create a new table, poi2, with the converted values for ease of use and simplicity. By taking the latitude/longitude coordinate and subtracting the lowest value in the data range (lat: 48.06, lon:11.358) we would then multiply by our magic number to get the value on our 0-1000 scale. This magic number was derived by dividing 1000 by the difference in our range. 

ie. 
M = 1000/(11.724-11.358) = 2732.240437 for longitude 
M = 1000/(48.249-48.06) = 5291.005291 for latitude.

From this number we then add or subtract another magic number that corresponds to 5 meters on our 0-1000 grid (to give us a bounding box of 10 meters). This magic number was found by determing the size of our range of lat/lon coordinates in meters (111319 meters per degree), and then translating that to our 0-1000 scale.

ie.
M = (1000/(111319*(48.249-48.06)))*5 = 0.2376506 for latitude
M = (1000/(111319*(11.724-11.358)))*5 = 0.1227212 for longitude