# commands can appear in any order
# comments begin with '#'
# blank lines are ignored
# all items in this file are optional and have program defaults

# levels define the characters to be dropped for that level of play
# note that the gletters keyboard is case-insensitive so the A key
# will "pop" both A and a, no shifting or mapping is necessary

level 1 АПРО
level 2 ВАПРОЛД
level 3 ЫВАПРОЛДЖ
level 4 ФЫВАПРОЛДЖЭ
level 5 ФЫВАПРОЛДЖЭГЕНТМИ
level 6 ФЫВАПРОЛДЖЭГЕНТМИКШУЩСЬЧБ
level 7 ФЫВАПРОЛДЖЭГЕНТМИКШУЩСЬЧБЗЦХЙ
level 8 ФЫВАПРОЛДЖЭГЕНТМИКШУЩСЬЧБЗЦХЙЯЧСЮБЬ
level 9 АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЭЮЯЪЬабвгдеёжзийклмнопрстуфхцчшщэюяъь0123456789

# key maps map keys on the keyboard to characters that are dropped
# so for example for this japanese keyboard, we map the lower case
# keys letters (a-z0-9) and some punctuation to hiragana and katakana 
# gcompris at this point does not recognize alternate input methods
# so kana input has to be done via mapping the romaji keyboard

key qй
key wц
key eу
key rк
key tе
key yн
key uг
key iш
key oщ
key pз
key [х
key ]ъ
key aф
key sы
key dв
key fа
key gп
key hр
key jо
key kл
key lд
key ;ж
key 'э
key zя
key xч
key cс
key vм
key bи
key nт
key mь
key ,б
key .ю

# the fallrate is the rate at which letters fall
# the smaller the numbers, the faster they fall
# the first number is the base
# the second number is the variability by level
# the sum of the numbers should be around 140
# use a slower fall rate for beginning typists
# use a faster rate to keep the challenge for more
# skilled typists

fallrate 40 100

# the droprate is the rate at which letters are dropped
# the smaller the numbers, the more letters drop
# the first number is the base
# the second number is the variability by level
# the sum of the numbers should be around 9000

droprate 1000 8000
