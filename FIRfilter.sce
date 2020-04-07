clear,xdel(winsid()),clc
f0=500
fm=10
Fm=fm/f0
L=52
typ='hn'
n=-L/2:L/2
hn=wfir('lp',L,[Fm 0],typ,[0 0])
[H1,F]=frmag(hn,1000)
hn=wfir('lp',L,[Fm 0],typ,[0 0])/H1(1)
disp(round(hn*2^15))

[H,F]=frmag(hn,1000)
scf()
subplot(211)
plot(hn,'o')
subplot(212)
plot(F*f0,H)

x=[ones(n)*1023 ones(n)*500 ones(n)*100 ones(n)*(-500)]
y=convol(x,hn)
scf()
plot(x,'ro')
plot(y,'o')

printf("	const int32_t b[]={");
for i=1:(L/2-1)
printf("%d, ",round(hn(i)*2^15));
end
printf("%d};",round(hn(i+1)*2^15));
