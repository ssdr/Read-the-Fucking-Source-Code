/*
HTTP1.1��CHUNKED�������
һ��HTTPͨ��ʱ����ʹ��Content-Lengthͷ��Ϣ����֪ͨ�û�������ͨ��������������������������͵��ĵ����ݳ��ȣ�
��ͷ��Ϣ������HTTP1.0Э��RFC  1945  10.4�½��С���������յ���ͷ��Ϣ�󣬽�����Content-Length�ж���ĳ�����
�ں�ʼ����ҳ�棬�����������в��������ӳٷ������������������������ɱȽ������û����顣

�����������HTTP1.1Э���У�RFC  2616��14.41�½��ж����Transfer-Encoding: chunked��ͷ��Ϣ��chunked���붨��
��3.6.1�У�����HTTP1.1 Ӧ�ö�֧�ִ�ʹ��trunked���붯̬���ṩbody���ݵĳ��ȵķ�ʽ������Chunked���봫���HTTP
����Ҫ����Ϣͷ�����ã�Transfer-Encoding: chunked��ʾContent Body����chunked���봫�����ݡ����ݶ��壬�������
��Ҫ�ȵ������ֽ�ȫ���������,ֻҪ���յ�һ��chunked��Ϳɽ���ҳ��.���ҿ�������html�ж����ҳ������,����js,css,image�ȡ�

����chunked����������ѡ��,һ�����趨Server��IO buffer������Server�Զ�flush buffer�е����ݣ���һ������
������IO�е�flush��������ͬ������IO�ж���flush���ܣ�

l         php:    ob_flush(); flush();

l         perl:   STDOUT->autoflush(1);

l         java:  out.flush();

l         python:  sys.stdout.flush()

l         ruby:  stdout.flush

����HTTP1.1��Transfer-Encoding:chunked,���Ұ�IO��buffer flush����,�Ա���������������ҳ��������Դ��������Ԥ��ȷ
��������ĳ���ʱ����������ͷ�а���Content-Length����ָ�������峤�ȣ���ʱ����Ҫͨ��Transfer-Encoding����ȷ��������
���ȡ�

Chunked����һ��ʹ�����ɸ�chunk�������ɣ������һ����������Ϊ0��chunk��ʾ������ÿ��chunk��Ϊͷ�������������֣�ͷ��
����ָ����һ�����ĵ��ַ����������㿪ͷ��ʮ�����Ƶ����֣���������λ��һ�㲻д,��ʾ�ֽڣ�.���Ĳ��־���ָ�����ȵ�ʵ��
���ݣ�������֮���ûس�����(CRLF)�����������һ������Ϊ0��chunk�е������ǳ�Ϊfooter�����ݣ���һЩ���ӵ�Header��Ϣ
��ͨ������ֱ�Ӻ��ԣ���

�������͹��ڹٷ��������֮��chunked����Ļ��������ǽ�������ݷֽ�ɶ��С���ݣ�ÿ�鶼������ָ�����ȣ�������ʽ��
��(BNF�ķ�)��

Chunked-Body   = *chunk            //0�����chunk
last-chunk         //���һ��chunk
trailer            //β��
CRLF               //������Ƿ�
chunk          = chunk-size [ chunk-extension ] CRLF
chunk-data CRLF
chunk-size     = 1*HEX
last-chunk     = 1*("0") [ chunk-extension ] CRLF
chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
chunk-ext-name = token
chunk-ext-val  = token | quoted-string
chunk-data     = chunk-size(OCTET)
trailer        = *(entity-header CRLF)
���ͣ�

l         Chunked-Body��ʾ����chunked�����ı����塣��������Է�Ϊchunk, last-chunk��trailer�ͽ������Ĳ��֡�chunk��
�����ڱ����������ٿ���Ϊ0�������ޣ�

l         ÿ��chunk�ĳ�������ָ���ģ�������ʼ�����ݱ�Ȼ��16�������ֵ��ַ�������������chunk-data�ĳ��ȣ��ֽ���������
��16���Ƶ��ַ�����һ���ַ�����ǡ�0�������ʾchunk-sizeΪ0����chunkΪlast-chunk,��chunk-data���֡�

l         ��ѡ��chunk-extension��ͨ��˫������ȷ������������߲������������壬���Ժ��ԡ�

l         trailer�Ǹ��ӵ���β���Ķ���ͷ��ͨ������һЩԪ���ݣ�metadata, meta means "about information"������Щͷ
������ڽ���󸽼�������ͷ��֮��

���������etherealץ��ʹ��Firefox��ĳ��վͨ�ŵĽ������ͷ���������ʼ����

Address  0..........................  f
000c0                       31
000d0    66 66 63 0d 0a ...............   // ASCII��:1ffc/r/n, chunk-data������ʼ��ַΪ000d5
��Ȼ����1ffc��Ϊ��һ��chunk��chunk-size,ת��ΪintΪ8188������1ffc�����Ͼ���CRLF,���û��chunk-extension��chunk-data
����ʼ��ַΪ000d5, �����֪��һ��chunk����ʼ
��ַΪ000d5+1ffc + 2=020d3�����£�
020d0    .. 0d 0a 31 66 66 63 0d 0a .... // ASCII��:/r/n1ffc/r/n
ǰһ��0d0a����һ��chunk�Ľ�����Ƿ�����һ��0d0a����chunk-size��chunk-data�ķָ�����
�˿�chunk�ĳ���ͬ��Ϊ8188, �������ƣ�ֱ�����һ��
100e0                          0d 0a 31
100f0    65 61 39 0d 0a......            //ASII�룺/r/n/1ea9/r/n
�˿鳤��Ϊ0x1ea9 = 7849, ��һ����ʼΪ100f5 + 1ea9 + 2 = 11fa0,���£�
11fa0    30 0d 0a 0d 0a                  //ASCII�룺0/r/n/r/n
��0��˵����ǰchunkΪlast-chunk, ��һ��0d 0aΪchunk���������ڶ���0d0a˵��û��trailer���֣�����Chunk-body������
�������̣�
��chunked������н����Ŀ���ǽ��ֿ��chunk-data���ϻָ���һ����Ϊ�����壬ͬʱ��¼�˿���ĳ��ȡ�
RFC2616�и����Ľ����������£�(α���룩
length := 0         //���ȼ�������0
read chunk-size, chunk-extension (if any) and CRLF      //��ȡchunk-size, chunk-extension��CRLF
while(chunk-size > 0 )  
 {            //��������last-chunk
read chunk-data and CRLF            //��chunk-size��С��chunk-data,skip CRLF
append chunk-data to entity-body     //���˿�chunk-data׷�ӵ�entity-body��
length := length + chunk-size
read chunk-size and CRLF          //��ȡ��chunk��chunk-size �� CRLF
}
read entity-header      //entity-header�ĸ�ʽΪname:valueCRLF,���Ϊ�ռ�ֻ��CRLF
while ��entity-header not empty)   //��������ֻ��CRLF�Ŀ���
{
append entity-header to existing header fields
read entity-header
}
Content-Length:=length      //�������������̽��������õ����±�����length����ΪContent-Length���ֵд�뱨����
Remove "chunked" from Transfer-Encoding  //ͬʱ��Transfer-Encoding����ֵȥ��chunked������
length����ֵʵ��Ϊ����chunk��chunk-size֮�ͣ��������ץ��ʵ���У�һ���а˿�chunk-sizeΪ0x1ffc(8188)��chunk,ʣ��һ��Ϊ0x1ea9(7849),������һ��73353�ֽڡ�
      ע����������������ǰ����chunk�Ĵ�С����8188,��������Ϊ:"1ffc" 4�ֽڣ�""r"n"2�ֽڣ����Ͽ�βһ��""r"n"2�ֽ�һ��8�ֽڣ����һ��chunk����Ϊ8196,���ÿ����Ƿ��Ͷ�һ��TCP���͵Ļ����С��
����ṩһ��PHP�汾��chunked������룺
$chunk_size = (integer)hexdec(fgets( $socket_fd, 4096 ) );
while(!feof($socket_fd) && $chunk_size > 0)
{
$bodyContent .= fread( $socket_fd, $chunk_size );
fread( $socket_fd, 2 ); // skip /r/n
    $chunk_size = (integer)hexdec(fgets( $socket_fd, 4096 ) );
}
 

 
��C���ԵĽ������£�java˼·��ͬ
int nBytes;
char* pStart = a;    // a�д�Ŵ����������
char* pTemp;
char strlength[10];   //һ��chunk��ĳ���
chunk  : pTemp =strstr(pStart,"/r/n");
             if(NULL==pTemp)
             {
                      free(a);
                 a=NULL;
                     fclose(fp);
                     return -1;
             }
             length=pTemp-pStart;
             COPY_STRING(strlength,pStart,length);
             pStart=pTemp+2;
             nBytes=Hex2Int(strlength); //�õ�һ����ĳ��ȣ���ת��Ϊʮ����
                              
             if(nBytes==0)//�������Ϊ0����Ϊ���һ��chunk
            {
                free(a);
                       fclose(fp);
                       return 0;
               }
               fwrite(pStart,sizeof(char),nBytes,fp);//��nBytes���ȵ�����д���ļ���
               pStart=pStart+nBytes+2; //����һ����������Լ�����֮�������ֽڵĽ�����
               fflush(fp);
               goto chunk; //goto��chunk��������
  
��ν�һ��ʮ������ת��Ϊʮ������
char *buf = (char *)malloc(100);
char *d = buf;
int shift = 0;
unsigned long copy = 123445677;
while (copy) {
         copy >>= 4;
         shift++;
}//���ȼ���ת��Ϊʮ�����ƺ��λ��
if (shift == 0)
         shift++;
shift <<= 2; //��λ������4���������λ�Ļ� shiftΪ8
while (shift > 0) {
         shift -= 4;
         *(buf) = hex_chars[(123445677 >> shift) & 0x0F];
          buf++;
}
*buf = '/0';



��ʽ:

ʮ������ea5���������ٿ���3749�ֽ�
          �����Ϊ3749�ֽڣ�����������\r\n����������Ѿ�����               �����Ϊ3752�ֽڣ�����������\r\n����������Ѿ����� 
                                                                                                                                 0��ʾ���һ���飬��������\r\n
ea5\r\n........................................................\r\n ea8\r\n..................................................\r\n 0\r\n\r\n



�ο�:http://blog.csdn.net/zhangboyj/article/details/6236780

*/
