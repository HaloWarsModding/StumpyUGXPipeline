fn ReadFixedString bstream fixedLen = (
   local str = ""
   For i = 1 to fixedLen do
   (
      str0 = ReadByte bstream #unsigned
      If str0!=0xFD AND str0!=0xFC do str+= bit.intAsChar str0
   )
   str
)
fn addHexaDecimalPadding nr = (
	case of (
		(nr <= 15): ("0" + ((bit.intAsHex(nr))as string))
		(nr > 15): ((bit.intAsHex(nr))as string)
	)
)	
fn ReadFixedByteString bstream fixedLen = (
   local str = ""
   For i = 1 to fixedLen do
   (
      str += addHexaDecimalPadding (ReadByte bstream #unsigned)
	  If mod i 1 == 0 do str += " "
   )
   str
   print str
)

fn convertTo32 input16 = (
      inputAsInt = input16
      sign = bit.get inputAsInt 16
      exponent = (bit.shift (bit.and inputAsInt (bit.hexasint "7C00")) -10) as integer - 16
      fraction = bit.and inputAsInt (bit.hexasint "03FF")
      if sign==true then sign = 1 else sign = 0
      exponentF = exponent + 127
      --Ouput 32 bit integer representing a 32 bit float
      outputAsFloat = bit.or (bit.or (bit.shift fraction 13) (bit.shift exponentF 23)) (bit.shift sign 31)
      --Output Check   
      return bit.intasfloat outputasfloat
)

fn floatSwap2 f = (
   i = bit.floatAsInt f
   h = bit.intashex i
   while h.count < 8 do h = "0" + h
   
   s = (substring h 7 2) + (substring h 5 2) + (substring h 3 2) + (substring h 1 2)
   bit.intAsFloat (bit.hexasint s)
)   
fn paddstring len instring = (
	instring=instring as string
	local str=""
	if instring.count <=len then (
		for i = 1 to (len-instring.count) do(
			str+="0"
		)
		str = (str+instring)
	)
	else (
		for i = 1 to len do(
			str+="0";str[i]=instring[i]
		)
	)
	str
)
fn printHex num = (
	print (bit.intashex(num))
)
fn printHere bstream = (
	Print ("Here @ 0x"+((bit.intAsHex(ftell bstream))as string))
)
fn getBoundBox bstream fscale = (
	currBBMax = ([(readfloat bstream),(readfloat bstream),(readfloat bstream)]*fscale)
	currBBMin = ([(readfloat bstream),(readfloat bstream),(readfloat bstream)]*fscale)
	currA = currBBMax - currBBMin
	
	bb = box()
	bb.width = abs(currA[1])
	bb.length = abs(currA[2])
	bb.height = abs(currA[3])
-- 	hide bb
	delete bb
)
fn getName bstream offset = (
	backJump = ftell bstream
	fseek bstream offset #seek_set
	_name = readstring bstream
	fseek bstream backJump #seek_set
	return _name
)
fn getOffset bstream offset = (
	backJump = ftell bstream
	fseek bstream offset #seek_set
	fseek bstream 0x4 #seek_cur
	newOffset = readlong bstream
	fseek bstream backJump #seek_set
	return newOffset
)
fn AlignOffset fstream relativeoffset alignment = (
   align = (alignment - (mod relativeoffset alignment as integer))
   align = fseek fstream align #seek_cur
   return align
)
fn ReadHalfFloat fstream = (
    hf=readshort fstream #unsigned
    sign = bit.get hf 16
    exponent = (bit.shift (bit.and hf (bit.hexasint "7C00")) -10) as integer - 16
    fraction = bit.and hf (bit.hexasint "03FF")
    if sign==true then sign = 1 else sign = 0
    exponentF = exponent + 127
    outputAsFloat = bit.or (bit.or (bit.shift fraction 13) \
    (bit.shift exponentF 23)) (bit.shift sign 31)
return bit.intasfloat outputasfloat*2
)
fn skipZeroes bstream = (
	char = 0
	while(char == 0) do (
		char = readbyte bstream #unsigned
	)
	fseek bstream -1 #seek_cur
)
fn reverseString s = (
    local reversed = ""
    for i in s.count to 1 by -1 do reversed += s[i]
	reversed
)
fn printBinary val size = (
	str = ""
	for i=1 to size do (
		if (bit.get val i)  then str += "1"
		else str += "0"
		
		if ((mod i 8) == 0) and (i < size) do str+=" "
	)
	
	str = reverseString str
	print str
)
fn ReadBEShort fstream = (
short = readshort fstream #unsigned
short = bit.swapBytes short 2 1
b = (bit.get short 16)
for i = 17 to 32 do short = bit.set short i b
return short
)
fn convertTo32 input16 = (
      inputAsInt = input16
      sign = bit.get inputAsInt 16
      exponent = (bit.shift (bit.and inputAsInt (bit.hexasint "7C00")) -10) as integer - 16
      fraction = bit.and inputAsInt (bit.hexasint "03FF")
      if sign==true then sign = 1 else sign = 0
      exponentF = exponent + 127
      --Ouput 32 bit integer representing a 32 bit float
      outputAsFloat = bit.or (bit.or (bit.shift fraction 13) (bit.shift exponentF 23)) (bit.shift sign 31)
      --Output Check   
      return bit.intasfloat outputasfloat
)
fn ReadBEHalfFloat fstream = (
return convertTo32(ReadBEword fstream)
)
fn floatSwap2 f = (
   i = bit.floatAsInt f
   h = bit.intashex i
   while h.count < 8 do h = "0" + h
   
   s = (substring h 7 2) + (substring h 5 2) + (substring h 3 2) + (substring h 1 2)
   bit.intAsFloat (bit.hexasint s)
)   
fn ReadBEword fstream = (
return (bit.swapBytes (readshort fstream #unsigned) 1 2)
)
fn ReadBElong fstream = (
long = readlong fstream
long = bit.swapBytes long 1 4
long = bit.swapBytes long 2 3
return long
)
fn ReadBEfloat fstream = (
return floatSwap2(readfloat fstream)
)

struct weight_data (
	boneids,weights
)

fname = GetOpenFileName caption:"Halo Wars 2" types:" (*.uax)|*.uax" historyCategory:"Halo Wars 2 Granny Animation"
-- fname=@"C:\Games\Halo Wars 2\AppFiles\data\dlc1\pack1\animation\unsc\infantry\flamecyclops01\flamecyclops_attack01.uax"
-- fname=@"C:\Users\Xavier\Documents\stuff\projects\models\ripped\halo wars 2\fatality_jumpbrute01_v_eliteranger01_01_victim.uax"
-- fname=@"C:\Users\Xavier\Documents\stuff\projects\models\ripped\halo wars 2\elitehon_combat_action_02.uax"
-- fname=@"C:\Users\Xavier\Documents\stuff\projects\models\ripped\halo wars 2\carbineelite_idle_to_run.uax"
-- fname=@"C:\Users\Xavier\Documents\stuff\projects\models\ripped\halo wars 2\data\animations\covenant\infantry\stormelitehonorguard01\elitehon_sword_attack_01.uax"
If (fname!=undefined) and ((DoesFileExist fname)==true) then (
f = fopen fname "rb"   --open file in read only format
actionMan.executeAction 0 "550"  -- Views: Viewport Visual Style Shaded
FileExtension = getFileNameType  fname
FileName = getFileNameFile fname
clearlistener()
Global fscale= 25
Print "===================================="
print (FileName + FileExtension) as string
Print "===================================="
--)

ScaleTable =#(
	1.4142135, 0.70710677, 0.35355338, 0.35355338,
	0.35355338, 0.17677669, 0.17677669, 0.17677669,
	-1.4142135, -0.70710677, -0.35355338, -0.35355338,
	-0.35355338, -0.17677669, -0.17677669, -0.17677669
)
OffsetTable =#(
	-0.70710677, -0.35355338, -0.53033006, -0.17677669,
	0.17677669, -0.17677669, -0.088388346, 0.0,
	0.70710677, 0.35355338, 0.53033006, 0.17677669,
	-0.17677669, 0.17677669, 0.088388346, -0.0
)

magic = readfixedstring f 4
tableOffset = readbelong f
ukw = readbelong f	--fileVer?
fileSize = readbelong f
tableCount = readbeshort f
null = readbeshort f
ukw2 = readbelong f

dataType=#()
dataOffset=#()
dataLength=#()
fseek f tableOffset #seek_set
for x=1 to tableCount do (
	null = readbelong f
	dataType[x] = readbelong f
	dataOffset[x] = readbelong f
	dataLength[x] = readbelong f
	ukw2 = for y=1 to 4 collect (readbeshort f)
)

for x=1 to tableCount do (
	subDataOffset=#()
	fseek f dataOffset[x] #seek_set
	
	if dataType[x] == 0x700 do (
		for y=1 to 13 do (
			flag = readlong f
			offset = readlong f + dataOffset[x]
			null = readlong f
			
			if flag == 1 do append subDataOffset offset
		)
		for y=1 to subDataOffset.count do (
			fseek f subDataOffset[y] #seek_set
			fseek f (readlong f + dataOffset[x]) #seek_set
			/*
			if y==1 do (	--boneData
				boneNameTable = readlong f + dataOffset[x]
				null = readlong f
				boneCount = readlong f
				boneOffset = readlong f + dataOffset[x]
				
				fseek f boneOffset #seek_set
				BNArr=#()
				for z=1 to boneCount do (
					boneName = getName f (readlong f + dataOffset[x])
					null = readlong f
					parentID = readlong f + 1
					siblingID = readlong f + 1
					fseek f 0x40 #seek_cur
					m11 = readfloat f; m12 = readfloat f; m13 = readfloat f; m14 = readfloat f
					m21 = readfloat f; m22 = readfloat f; m23 = readfloat f; m24 = readfloat f
					m31 = readfloat f; m32 = readfloat f; m33 = readfloat f; m34 = readfloat f
					m41 = readfloat f; m42 = readfloat f; m43 = readfloat f; m44 = readfloat f
					tfm = matrix3 [m11,m21,m31] [m12,m22,m32] [m13,m23,m33] [m14,m24,m34]	
					ukw = readshort f
					ukw2 = readshort f
					fseek f 0x10 #seek_cur
					
					newBone = bonesys.createbone	\
					tfm.row4	\
					(tfm.row4 + 0.01 * (normalize tfm.row1)) \
					(normalize tfm.row3)
					newBone.width = 0.1
					newBone.height = 0.1
		 			newBone.wirecolor = orange
					newbone.showlinks = true
					newBone.transform = tfm
					newBone.name = boneName
					
					pos = [m41,m42,m43]
					pos *= tfm
					newBone.pos += ((fscale)*-pos)
					
					newBone.setBoneEnable false 0
					newBone.pos.controller = TCB_position ()
					newBone.rotation.controller = TCB_rotation ()
 					
					if (parentID != 0) then (
						newBone.parent = BNArr[parentID]
					)
 					
					append BNArr newBone
				)
				for z in BNArr do (z.setskinpose())
				for z in BNArr where z.children.count == 0 do z.wirecolor = green
			)
			*/
			if y==1 do (	--animationData
				rootBone = getName f (readlong f + dataOffset[x])
				fseek f 0x10 #seek_cur	--null
				
				boneCount = readlong f
				controlArrOffset = readlong f + dataOffset[x]
				null = readlong f
				
				boneCount2 = readlong f	--for larger animations is null
				ukwOffset2 = readlong f + dataOffset[x]
				null = readlong f
				
				fseek f 0x0c #seek_cur	--null
				ukw = readlong f			--comType?
				fseek f 0x1c #seek_cur	--unknown
				
				posCurveTypeArr=#()
				rotCurveTypeArr=#()
				sclCurveTypeArr=#()
				
				numFrames = 0
				sliderTime = 0
				deleteKeys objects #allKeys
				allBones = for o in geometry where classOf o == boneGeometry collect o
				for a in allBones do a.assumeskinpose()
				
				fseek f controlArrOffset #seek_set
				for z=1 to boneCount do (
					boneName = getName f (readlong f + dataOffset[x])
					null = readlonglong f
					
					rotCurveType = getName f (getOffset f (readlonglong f #unsigned + dataOffset[x]) + dataOffset[x])
					rotDataOffset = readlonglong f #unsigned + dataOffset[x]
					
					posCurveType = getName f (getOffset f (readlonglong f #unsigned + dataOffset[x]) + dataOffset[x])
					posDataOffset = readlonglong f #unsigned + dataOffset[x]
					
					sclCurveType = getName f (getOffset f (readlonglong f #unsigned + dataOffset[x]) + dataOffset[x])
					sclDataOffset = readlonglong f #unsigned + dataOffset[x]
					
					backJump = ftell f
					
					--Position Data--
					fseek f posDataOffset #seek_set
					appendifunique posCurveTypeArr posCurveType
					
					if posCurveType == "CurveDataHeader_DaK32fC32f" do (
						curveDataHeader = readshort f #unsigned
						padding = readshort f #unsigned
						knotCount = readlong f #unsigned
						knotOffset = readlong f #unsigned + dataOffset[x]
						null = readlong f #unsigned
						controlCount = readlong f #unsigned
						controlOffset = readlong f #unsigned + dataOffset[x]
						null = readlong f #unsigned
						
						fseek f knotOffset #seek_set
						knotArr = for a=1 to knotCount collect (readfloat f)
						
						fseek f controlOffset #seek_set
						controlArr = for a=1 to knotCount collect (point3 (readfloat f) (readfloat f) (readfloat f))
						
						with animate on(
							for a=1 to knotCount do (
								
								b = getnodebyname boneName
								t = knotArr[a] * framerate
								p = controlArr[a]
								
								if (b!=undefined) then (
									at time t
									b.Controller.Position = p
								)
								
								if t > numFrames do numFrames = t
							)
						)
					)
					if posCurveType == "CurveDataHeader_D3Constant32f" do (
						curveDataHeader = readshort f #unsigned
						padding = readshort f #unsigned
						
						with animate on(
							for a=1 to 1 do (
								
								b = getnodebyname boneName
								t = 0
								p = (point3 (readfloat f) (readfloat f) (readfloat f))*fscale
								
								if (b!=undefined) then (
									at time t
									b.Controller.Position = p
								)
							)
						)
					)
					if posCurveType == "CurveDataHeader_D3K16uC16u" do (
-- 						ReadFixedByteString f 0x24
-- 						fseek f -0x24 #seek_cur
						
						curveDataHeader = readshort f
						OneOverKnotScaleTrunc = readshort f #unsigned
						controls = for x=1 to 3 collect (readfloat f)
						offsets = for x=1 to 3 collect (readfloat f)
						knotControlCount = readlong f #unsigned
						knotsControlOffset = readlong f #unsigned + dataOffset[x]
						
						knotCount = knotControlCount / 4
						fseek f KnotsControlOffset #seek_set
						
						knotArr=#()
						for x=1 to knotCount do (
							knotData = (readshort f #unsigned) as float
							knotArr[x] = (knotData/(bit.intasfloat ((bit.shift OneOverKnotScaleTrunc 16))))
						)
						
						controlArr=#()
						for x=1 to knotCount do (
							px = (readshort f #unsigned) * controls[1] + offsets[1]
							py = (readshort f #unsigned) * controls[2] + offsets[2]
							pz = (readshort f #unsigned) * controls[3] + offsets[3]
							
							append controlArr([px,py,pz]*fscale)
						)
						
						with animate on(
							for a=1 to knotCount do (
								
								b = getnodebyname boneName
								t = knotArr[a] * framerate
								p = controlArr[a]
								
								if (b!=undefined) then (
									at time t
									b.Controller.Position = p
								)
								
								if t > numFrames do numFrames = t
							)
						)						
					)
					if posCurveType == "CurveDataHeader_D3I1K16uC16u" do (
-- 						ReadFixedByteString f 0x24
-- 						fseek f -0x24 #seek_cur
						
						curveDataHeader = readshort f
						OneOverKnotScaleTrunc = readshort f #unsigned
						controls = for x=1 to 3 collect (readfloat f)
						offsets = for x=1 to 3 collect (readfloat f)
						knotControlCount = readlong f #unsigned
						knotsControlOffset = readlong f #unsigned + dataOffset[x]
						
						knotCount = knotControlCount / 2
						fseek f KnotsControlOffset #seek_set
						
						knotArr=#()
						for x=1 to knotCount do (
							knotData = (readshort f #unsigned) as float
							knotArr[x] = (knotData/(bit.intasfloat ((bit.shift OneOverKnotScaleTrunc 16))))
						)
						
						controlArr=#()
						for x=1 to knotCount do (
							p = readshort f #unsigned
							px = p * controls[1] + offsets[1]
							py = p * controls[2] + offsets[2]
							pz = p * controls[3] + offsets[3]
							
							append controlArr([px,py,pz]*fscale)
						)
						
						with animate on(
							for a=1 to knotCount do (
								
								b = getnodebyname boneName
								t = knotArr[a] * framerate
								p = controlArr[a]
								
								if (b!=undefined) then (
									at time t
									b.Controller.Position = p
								)
								
								if t > numFrames do numFrames = t
							)
						)
					)
					if posCurveType == "CurveDataHeader_D3K8uC8u" do (
-- 						ReadFixedByteString f 0x24
-- 						fseek f -0x24 #seek_cur
						
						curveDataHeader = readshort f
						OneOverKnotScaleTrunc = readshort f #unsigned
						controls = for x=1 to 3 collect (readfloat f)
						offsets = for x=1 to 3 collect (readfloat f)
						knotControlCount = readlong f
						knotsControlOffset = readlong f + dataOffset[x]
						
						knotCount = knotControlCount / 4
						fseek f KnotsControlOffset #seek_set
						
						knotArr=#()
						for x=1 to knotCount do (
							knotData = (readbyte f #unsigned) as float
							knotArr[x] = (knotData/(bit.intasfloat ((bit.shift OneOverKnotScaleTrunc 16))))
						)
						
						controlArr=#()
						for x=1 to knotCount do (
							px = (readbyte f #unsigned) * controls[1] + offsets[1]
							py = (readbyte f #unsigned) * controls[2] + offsets[2]
							pz = (readbyte f #unsigned) * controls[3] + offsets[3]
							
							append controlArr([px,py,pz]*fscale)
						)
						
						with animate on(
							for a=1 to knotCount do (
								
								b = getnodebyname boneName
								t = knotArr[a] * framerate
								p = controlArr[a]
								
								if (b!=undefined) then (
									at time t
									b.Controller.Position = p
								)
								
								if t > numFrames do numFrames = t
							)
						)
					)
					if posCurveType == "CurveDataHeader_D3I1K8uC8u" do (
-- 						ReadFixedByteString f 0x24
-- 						fseek f -0x24 #seek_cur
						
						curveDataHeader = readshort f #unsigned
						OneOverKnotScaleTrunc = readshort f #unsigned
						controls = for x=1 to 3 collect (readfloat f)
						offsets = for x=1 to 3 collect (readfloat f)
						knotControlCount = readlong f #unsigned
						knotsControlOffset = readlong f #unsigned + dataOffset[x]
						
						knotCount = knotControlCount / 2
						fseek f KnotsControlOffset #seek_set
						
						knotArr=#()
						for x=1 to knotCount do (
							knotData = (readbyte f #unsigned) as float
							knotArr[x] = (knotData/(bit.intasfloat ((bit.shift OneOverKnotScaleTrunc 16))))
						)
						
						controlArr=#()
						for x=1 to knotCount do (
							p = readbyte f #unsigned
							px = p * controls[1] + offsets[1]
							py = p * controls[2] + offsets[2]
							pz = p * controls[3] + offsets[3]
							
							append controlArr([px,py,pz]*fscale)
						)
						
						with animate on(
							for a=1 to knotCount do (
								
								b = getnodebyname boneName
								t = knotArr[a] * framerate
								p = controlArr[a]
								
								if (b!=undefined) then (
									at time t
									b.Controller.Position = p
								)
								
								if t > numFrames do numFrames = t
							)
						)
					)
					if posCurveType == "CurveDataHeader_DaIdentity" do (
					
					)
					
					--Rotation Data--
					fseek f rotDataOffset #seek_set					
					appendifunique rotCurveTypeArr rotCurveType
					
					if rotCurveType == "CurveDataHeader_DaK32fC32f" do (
						curveDataHeader = readshort f #unsigned
						padding = readshort f #unsigned
						knotCount = readlong f #unsigned
						knotOffset = readlong f + dataOffset[x]
						null = readlong f #unsigned
						controlCount = readlong f #unsigned
						controlOffset = readlong f #unsigned + dataOffset[x]
						null = readlong f
						
						fseek f knotOffset #seek_set
						knotArr = for a=1 to knotCount collect (readfloat f)
						
						fseek f controlOffset #seek_set
						controlArr = for a=1 to knotCount collect (quat (readfloat f) (readfloat f) (readfloat f) (readfloat f))
						
						with animate on(
							for a=1 to knotCount do (
								
								b = getnodebyname boneName
								t = knotArr[a] * framerate
								q = controlArr[a]
								
								if (b!=undefined) then (
									at time t
									b.Controller.Rotation = inverse q
								)
								
								if t > numFrames do numFrames = t
							)
						)
					)
					if rotCurveType == "CurveDataHeader_D4Constant32f" do (
						curveDataHeader = readshort f #unsigned
						padding = readshort f #unsigned
						
						with animate on(
							for a=1 to 1 do (
								
								b = getnodebyname boneName
								q = (quat (readfloat f) (readfloat f) (readfloat f) (readfloat f))
								
								if (b!=undefined) then (
									at time 0
									b.Controller.Rotation = inverse q
								)
							)
						)						
					)
					if rotCurveType == "CurveDataHeader_D4nK16uC15u" do (
 						curveDataHeader = readshort f #unsigned
						ScaleOffsetTableEntries = readshort f #unsigned
						OneOverKnotScale = readfloat f
						knotControlCount = readlong f #unsigned
						knotsControlOffset = readlong f #unsigned + dataOffset[x]
						
						knotCount = knotControlCount / 4
						fseek f KnotsControlOffset #seek_set
						
						selector = ScaleOffsetTableEntries
						
						scales=#(
							(ScaleTable[(bit.and (bit.shift selector 0) 0x0f) + 1] * 0.000030518509),
							(ScaleTable[(bit.and (bit.shift selector -4) 0x0f) + 1] * 0.000030518509),
							(ScaleTable[(bit.and (bit.shift selector -8) 0x0f) + 1] * 0.000030518509),
							(ScaleTable[(bit.and (bit.shift selector -12) 0x0f) + 1] * 0.000030518509)
						)
						offsets=#(
							(OffsetTable[(bit.and (bit.shift selector 0) 0x0f) + 1]),
							(OffsetTable[(bit.and (bit.shift selector -4) 0x0f) + 1]),
							(OffsetTable[(bit.and (bit.shift selector -8) 0x0f) + 1]),
							(OffsetTable[(bit.and (bit.shift selector -12) 0x0f) + 1])
						)
						
						knotArr=#()
						for x=1 to knotCount do (
							knotData = readshort f #unsigned
							knotArr[x] = (knotData / OneOverKnotScale)
						)
						
						controlArr=#()
						for x=1 to knotCount do (
							a = readShort f #unsigned
							b = readShort f #unsigned
							c = readShort f #unsigned
							
							swizzle1 = (bit.or (bit.shift (bit.and b 0x8000) -14) (bit.shift c -15))
							swizzle2 = (bit.and (swizzle1 + 1) 3)
							swizzle3 = (bit.and (swizzle2 + 1) 3)
							swizzle4 = (bit.and (swizzle3 + 1) 3)
							
							dataA = (bit.and a 0x7fff) * scales[swizzle2+1] + offsets[swizzle2+1]
							dataB = (bit.and b 0x7fff) * scales[swizzle3+1] + offsets[swizzle3+1]
							dataC = (bit.and c 0x7fff) * scales[swizzle4+1] + offsets[swizzle4+1]
 							
							dataD = sqrt(1.0 - (dataA*dataA + dataB*dataB + dataC*dataC))
							if ((bit.and a 0x8000) != 0) do (
								dataD = -dataD
							)
 							
							q = #(0,0,0,0)
							q[swizzle2 + 1] = dataA
							q[swizzle3 + 1] = dataB
							q[swizzle4 + 1] = dataC
							q[swizzle1 + 1] = dataD
 							
							controlArr[x] = quat q[1] q[2] q[3] q[4]
						)
						
						with animate on(
							for a=1 to knotCount do (
								
								b = getnodebyname boneName
								t = knotArr[a] * framerate
								q = controlArr[a]
								
								if (b!=undefined) then (
									at time t
									b.Controller.Rotation = inverse q
								)
								
								if t > numFrames do numFrames = t
							)
						)
					)
					if rotCurveType == "CurveDataHeader_D4nK8uC7u" do (
 						curveDataHeader = readshort f #unsigned
						ScaleOffsetTableEntries = readshort f #unsigned
						OneOverKnotScale = readfloat f 
						knotControlCount = readlong f #unsigned
						knotsControlOffset = readlong f + dataOffset[x]
						
						knotCount = knotControlCount / 4
						fseek f KnotsControlOffset #seek_set
						
						selector = ScaleOffsetTableEntries
						
						scales=#(
							(ScaleTable[(bit.and (bit.shift selector 0) 0x0f) + 1] * 0.0078740157),
							(ScaleTable[(bit.and (bit.shift selector -4) 0x0f) + 1] * 0.0078740157),
							(ScaleTable[(bit.and (bit.shift selector -8) 0x0f) + 1] * 0.0078740157),
							(ScaleTable[(bit.and (bit.shift selector -12) 0x0f) + 1] * 0.0078740157)
						)
						offsets=#(
							(OffsetTable[(bit.and (bit.shift selector 0) 0x0f) + 1]),
							(OffsetTable[(bit.and (bit.shift selector -4) 0x0f) + 1]),
							(OffsetTable[(bit.and (bit.shift selector -8) 0x0f) + 1]),
							(OffsetTable[(bit.and (bit.shift selector -12) 0x0f) + 1])
						)
						
						knotArr=#()
						for x=1 to knotCount do (
							knotData = readbyte f #unsigned
							knotArr[x] = (knotData / OneOverKnotScale)
						)
						
						controlArr=#()
						for x=1 to knotCount do (
							a = readbyte f #unsigned
							b = readbyte f #unsigned
							c = readbyte f #unsigned
							
							swizzle1 = (bit.or (bit.shift (bit.and b 0x80) -6) (bit.shift (bit.and c 0x80) -7))
							swizzle2 = (bit.and (swizzle1 + 1) 3)
							swizzle3 = (bit.and (swizzle2 + 1) 3)
							swizzle4 = (bit.and (swizzle3 + 1) 3)
							
							dataA = (bit.and a 0x7f) * scales[swizzle2+1] + offsets[swizzle2+1]
							dataB = (bit.and b 0x7f) * scales[swizzle3+1] + offsets[swizzle3+1]
							dataC = (bit.and c 0x7f) * scales[swizzle4+1] + offsets[swizzle4+1]
 							
							dataD = sqrt(1.0 - (dataA*dataA + dataB*dataB + dataC*dataC))
							if ((bit.and a 0x80) != 0) do (
								dataD = -dataD
							)
 							
							q = #(0,0,0,0)
							q[swizzle2 + 1] = dataA
							q[swizzle3 + 1] = dataB
							q[swizzle4 + 1] = dataC
							q[swizzle1 + 1] = dataD
							
							controlArr[x] = quat q[1] q[2] q[3] q[4]
						)
						
						with animate on(
							for a=1 to knotCount do (
								
								b = getnodebyname boneName
								t = knotArr[a] * framerate
								q = controlArr[a]
								
								if (b!=undefined) then (
									at time t
									b.Controller.Rotation = inverse q
								)
								
								if t > numFrames do numFrames = t
							)
						)
					)
					if rotCurveType == "CurveDataHeader_DaIdentity" do (
					
					)
					
					
					--Scale Data--
					fseek f sclDataOffset #seek_set		
					appendifunique sclCurveTypeArr sclCurveType
					
					fseek f backJump #seek_set
				)
				
				animationRange = interval 0 numFrames
				stopAnimation()
				
				format "Position Curve Types: \n"
				for a in posCurveTypeArr do print a
				format "\n" 
				
				format "Rotation Curve Types: \n"
				for a in rotCurveTypeArr do print a
				format "\n"
				
				format "Scale Curve Types: \n"
				for a in sclCurveTypeArr do print a
				format "\n"
			)
		)
	)
)

Print ("Last Read @ 0x"+((bit.intAsHex(ftell f))as string))
)
gc()
fclose f