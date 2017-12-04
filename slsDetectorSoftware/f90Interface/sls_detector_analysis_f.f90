module ax
USE,INTRINSIC :: ISO_C_BINDING
IMPLICIT NONE
integer(C_LONG) :: numberofmodules,totalnumberofchannels
real(C_DOUBLE) :: numberofbins
real(C_DOUBLE) :: bincenter(360000), binvalue(360000), binerror(360000) 

contains


subroutine initdataset(nmodules, chpmod, modulemask,badchanmask,ffcoeff,fferr,tdead,angradius,angoffset,angcenter, totaloffset, binsize, samplex, sampley) bind(c, name='init_dataset')
IMPLICIT NONE
integer(C_LONG), intent(IN) :: nmodules, chpmod
integer(C_LONG), intent(IN) :: modulemask(nmodules), badchanmask(nmodules*chpmod)
real(C_DOUBLE), intent(IN) :: ffcoeff(nmodules*chpmod), fferr(nmodules*chpmod)
real(C_DOUBLE), intent(IN) :: tdead, totaloffset,binsize, samplex, sampley
real(C_DOUBLE), intent(IN) :: angradius(nmodules), angoffset(nmodules), angcenter(nmodules)
integer(C_LONG) :: i

print*,'init dataset'

numberofmodules=nmodules

print*,'Number of modules:'
print*,numberofmodules
print*,'Channels per module:'
print*,chpmod

totalnumberofchannels=nmodules*chpmod

print*,'Total number of channels:'
print*,totalnumberofchannels


print*,'Modulemask:'
do i=1,nmodules
  print*,i,modulemask(i)
enddo

print*,'Badchannelmask:'
do i=1,nmodules*chpmod
  print*,i,badchanmask(i)
enddo

print*,'Flat field coefficients:'
do i=1,nmodules*chpmod
  print*,i,ffcoeff(i),'+-',fferr(i)
enddo

print*,'Tdead:'
print*,tdead


print*,'Angular conversion coefficients:'
do i=1,nmodules
  print*,i,angradius(i),angoffset(i),angcenter(i)
enddo


print*,'Total  offset:'
print*,totaloffset

print*,'Bin size:'
print*,binsize

numberofbins=360./binsize


print*,'Sample displacement:'
print*,samplex, sampley

end subroutine initdataset



subroutine resetdataset() bind(c, name='resetDataset')
IMPLICIT NONE

print*,'reset dataset'


end subroutine resetdataset


subroutine finalizedataset(outang, outval, outerr, outnpoints) bind(c, name='finalize_dataset')
IMPLICIT NONE
integer(C_LONG), intent(OUT) :: outnpoints
real(C_DOUBLE), intent(OUT) :: outang(*), outval(*), outerr(*)
integer(C_LONG) :: i

print*,'finalize dataset'

outnpoints=numberofbins

print*,'returning points:'
do i=1,numberofbins

   outang(i)=i;
   outval(i)=i*100;
   outerr(i)=i*0.1

  print*,i,outang(i),outval(i),outerr(i)
enddo

end subroutine finalizedataset



subroutine addframe(data, pos, i0, exptime, fname, var) bind(c, name='add_frame')
IMPLICIT NONE
real(C_DOUBLE), intent(IN) :: data(totalnumberofchannels)
real(C_DOUBLE), intent(IN) :: pos, i0, exptime, var
character(kind=c_char), dimension(*), intent(IN) :: fname
integer :: l
integer :: i

l=0
do
   if (fname(l+1) == C_NULL_CHAR) exit
   l = l + 1
end do


print*,'add frame'

print*,'Filename: '
print*,fname(1:l)

print*,'Position: '
print*,pos

print*,'I0: '
print*,i0

print*,'Exposure time: '
print*,exptime

print*,'Var: '
print*,var

print*,'Data: '
print*,data


end subroutine addframe



subroutine calculateflatfield(nmodules, chpmod, modulemask, badchanmask,data, ffcoeff,fferr) bind(c, name='calculate_flat_field')

IMPLICIT NONE
integer(C_LONG), intent(IN) :: nmodules, chpmod
integer(C_LONG), intent(IN) :: modulemask(nmodules), badchanmask(nmodules*chpmod)
real(C_DOUBLE), intent(IN) :: data(nmodules*chpmod)
real(C_DOUBLE), intent(OUT) ::  ffcoeff(*),fferr(*)
integer(C_LONG) :: i

real(C_DOUBLE) :: ave;

print*,'calculate flat field'

do i=1,nmodules*chpmod
   ffcoeff(i)=data(i)*10.
   fferr(i)=i
   print*,i,data(i), ffcoeff(i),fferr(i)
enddo




end subroutine calculateflatfield


end module ax
