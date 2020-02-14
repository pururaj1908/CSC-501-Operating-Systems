
typedef int WORD;
extern WORD _etext,_edata,_end;
void printsegaddress()
{
	int *etextptr,*edataptr,*eendptr;
	etextptr = &_etext;
	edataptr = &_edata;
	eendptr = &_end;
	kprintf("\n\nvoid printsegaddress()");
	kprintf("\nCurrent: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x",etextptr,*etextptr,edataptr,*edataptr,eendptr,*eendptr);


	kprintf("\nPreceding: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x",etextptr-1,*(etextptr - 1),edataptr-1,*(edataptr-1),eendptr-1,*(eendptr-1));
			

	kprintf("\nAfter: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x",etextptr+1,*(etextptr+1),edataptr+1,*(edataptr+1),eendptr+1,*(eendptr+1));

}
