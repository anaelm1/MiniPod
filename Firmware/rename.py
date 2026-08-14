import os
import csv
import shutil

#Initializing 
processed_rows = []
n = 0
Dict = "songs_dict.csv"

#Reading all files in song
pathIN = "D:/CS related/MP3 player/Firmware/Songs"
pathOUT = "D:/CS related/MP3 player/Firmware/Output"
dir_list = os.listdir(pathIN) 

for songs in dir_list: 
    name, ext = os.path.splitext(songs)
    if ext == '.mp3': #Check for song
        clean_name = name.removesuffix(".mp3")
        if " - " in clean_name: #dividing 
            artist, title = clean_name.split(" - ", 1)
        else:
            artist = "Unknown"
            title = clean_name

        index_str = f"{n:03d}" #indexing
        processed_rows.append([index_str, title.strip(), artist.strip()])   #adding to list    
        old_path = os.path.join(pathIN, songs)
        if os.path.isfile(old_path): #copy and rename
            new_path = os.path.join(pathOUT, f"{index_str}.mp3")
            shutil.copy(old_path, new_path)

        n += 1

with open("index.csv", "w", newline = "") as csvfile: #uploading to the csv file
    writer = csv.writer(csvfile)
    writer.writerow(["Index", "Title", "Artist"])
    writer.writerows(processed_rows)