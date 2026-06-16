import os

replacements = {
    '"#10161a"': '"#111111"',
    '"#162127"': '"#181818"',
    '"#0e1519"': '"#131313"',
    '"#10181d"': '"#1c1c1c"',
    '"#141c22"': '"#181818"',
    '"#182229"': '"#1c1c1c"',
    '"#2f4650"': '"#333333"',
    '"#10171c"': '"#111111"',
    '"#0a1014"': '"#0f0f0f"',
    '"#141e24"': '"#181818"',
    '"#1d3038"': '"#222222"',
    '"#2c4651"': '"#333333"'
}

count = 0
for root, dirs, files in os.walk('ui'):
    for file in files:
        if file.endswith('.qml'):
            path = os.path.join(root, file)
            with open(path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            new_content = content
            for old_c, new_c in replacements.items():
                new_content = new_content.replace(old_c, new_c)
                
            if new_content != content:
                with open(path, 'w', encoding='utf-8') as f:
                    f.write(new_content)
                count += 1
                print(f'Updated {path}')
print(f'Done updating {count} files.')
