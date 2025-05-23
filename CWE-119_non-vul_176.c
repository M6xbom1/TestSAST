static int set_reg_umr_segment(struct mlx5_ib_dev *dev,
			       struct mlx5_wqe_umr_ctrl_seg *umr,
			       const struct ib_send_wr *wr, int atomic)
{
	const struct mlx5_umr_wr *umrwr = umr_wr(wr);

	memset(umr, 0, sizeof(*umr));

	if (wr->send_flags & MLX5_IB_SEND_UMR_FAIL_IF_FREE)
		umr->flags = MLX5_UMR_CHECK_FREE; /* fail if free */
	else
		umr->flags = MLX5_UMR_CHECK_NOT_FREE; /* fail if not free */

	umr->xlt_octowords = cpu_to_be16(get_xlt_octo(umrwr->xlt_size));
	if (wr->send_flags & MLX5_IB_SEND_UMR_UPDATE_XLT) {
		u64 offset = get_xlt_octo(umrwr->offset);

		umr->xlt_offset = cpu_to_be16(offset & 0xffff);
		umr->xlt_offset_47_16 = cpu_to_be32(offset >> 16);
		umr->flags |= MLX5_UMR_TRANSLATION_OFFSET_EN;
	}
	if (wr->send_flags & MLX5_IB_SEND_UMR_UPDATE_TRANSLATION)
		umr->mkey_mask |= get_umr_update_translation_mask();
	if (wr->send_flags & MLX5_IB_SEND_UMR_UPDATE_PD_ACCESS) {
		umr->mkey_mask |= get_umr_update_access_mask(atomic);
		umr->mkey_mask |= get_umr_update_pd_mask();
	}
	if (wr->send_flags & MLX5_IB_SEND_UMR_ENABLE_MR)
		umr->mkey_mask |= get_umr_enable_mr_mask();
	if (wr->send_flags & MLX5_IB_SEND_UMR_DISABLE_MR)
		umr->mkey_mask |= get_umr_disable_mr_mask();

	if (!wr->num_sge)
		umr->flags |= MLX5_UMR_INLINE;

	return umr_check_mkey_mask(dev, be64_to_cpu(umr->mkey_mask));
}